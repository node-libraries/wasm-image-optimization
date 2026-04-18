#include "image_converter_api.h"
#include "utils/thumbhash.h"
#include <vector>
#include <emscripten.h>
#include <cstring>
#include <algorithm>

#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedOrigin.h"
#include "include/codec/SkPixmapUtils.h"
#include "include/codec/SkPngDecoder.h"
#include "include/codec/SkJpegDecoder.h"
#include "include/codec/SkWebpDecoder.h"
#include "include/codec/SkGifDecoder.h"
#include "include/codec/SkAvifDecoder.h"
#include "include/codec/SkBmpDecoder.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "include/codec/SkEncodedImageFormat.h"
#include "include/encode/SkWebpEncoder.h"
#include "include/encode/SkPngEncoder.h"
#include "include/encode/SkJpegEncoder.h"

// SVG support
#include "include/core/SkFontMgr.h"
#include "modules/skresources/include/SkResources.h"
#include "modules/svg/include/SkSVGDOM.h"
#include "modules/svg/include/SkSVGNode.h"
#include "include/core/SkStream.h"

// For libavif encoder
#include "avif/avif.h"

#include <cstdio>

#include <string>
#include <string_view>
#include <ctre.hpp>

extern "C" {
EM_JS(void, js_on_log, (int level, const char* message), {
    const msg = UTF8ToString(message);
    if (typeof onLog === 'function') {
        onLog(level, msg);
    } else if (typeof Module !== 'undefined' && typeof Module.onLog === 'function') {
        Module.onLog(level, msg);
    } else {
        // Fallback to console if no handler is found
        if (level <= 1) console.error(msg);
        else if (level == 2) console.warn(msg);
        else console.log(msg);
    }
})
}

void image_converter_log(LogLevel level, const char* message) {
    js_on_log((int)level, message);
}

// --- Logging ---
static int g_log_level = 0;

ImageConverterInstance::ImageConverterInstance() { context.init(); }

ImageConverterInstance::~ImageConverterInstance() {}

bool ImageConverterInstance::load_image(const uint8_t* data, size_t size) {
    frames.clear();
    original_data = SkData::MakeWithCopy(data, size);
    original_width = 0;
    original_height = 0;
    original_animation = false;
    original_format = "unknown";
    auto sk_data = original_data;

    // Try SkCodec (raster images and animations)
    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(sk_data);
    if (codec) {
        int frameCount = codec->getFrameCount();
        SkEncodedOrigin origin = codec->getOrigin();
        bool swapDimensions = SkEncodedOriginSwapsWidthHeight(origin);

        SkImageInfo info = codec->getInfo().makeColorType(kN32_SkColorType).makeAlphaType(kPremul_SkAlphaType);
        
        if (swapDimensions) {
            original_width = info.height();
            original_height = info.width();
        } else {
            original_width = info.width();
            original_height = info.height();
        }
        original_animation = frameCount > 1;

        auto encoded_format = codec->getEncodedFormat();
        switch (encoded_format) {
            case SkEncodedImageFormat::kPNG: original_format = "png"; break;
            case SkEncodedImageFormat::kJPEG: original_format = "jpeg"; break;
            case SkEncodedImageFormat::kWEBP: original_format = "webp"; break;
            case SkEncodedImageFormat::kGIF: original_format = "gif"; break;
            case SkEncodedImageFormat::kAVIF: original_format = "avif"; break;
            case SkEncodedImageFormat::kBMP: original_format = "bmp"; break;
            default: original_format = "unknown"; break;
        }

        for (int i = 0; i < frameCount; ++i) {
            SkBitmap decodedBitmap;
            decodedBitmap.allocPixels(info);

            SkCodec::Options options;
            options.fFrameIndex = i;
            
            if (codec->getPixels(info, decodedBitmap.getPixels(), decodedBitmap.rowBytes(), &options) == SkCodec::kSuccess) {
                SkCodec::FrameInfo frameInfo;
                codec->getFrameInfo(i, &frameInfo);

                if (origin != kDefault_SkEncodedOrigin) {
                    SkBitmap orientedBitmap;
                    SkImageInfo orientedInfo = info;
                    if (swapDimensions) {
                        orientedInfo = orientedInfo.makeWH(info.height(), info.width());
                    }
                    orientedBitmap.allocPixels(orientedInfo);
                    if (SkPixmapUtils::Orient(orientedBitmap.pixmap(), decodedBitmap.pixmap(), origin)) {
                        frames.push_back({std::move(orientedBitmap), frameInfo.fDuration});
                    } else {
                        frames.push_back({std::move(decodedBitmap), frameInfo.fDuration});
                    }
                } else {
                    frames.push_back({std::move(decodedBitmap), frameInfo.fDuration});
                }
            }
        }
        return !frames.empty();
    }

    // Try SVG
    SkMemoryStream stream(sk_data);
    auto resource_provider = skresources::DataURIResourceProviderProxy::Make(
        nullptr, skresources::ImageDecodeStrategy::kLazyDecode);
    auto svg_dom = SkSVGDOM::Builder()
                       .setResourceProvider(std::move(resource_provider))
                       .setFontManager(SkFontMgr::RefEmpty())
                       .make(stream);
    if (svg_dom) {
        auto container_size = svg_dom->containerSize();
        if (container_size.isEmpty()) {
            container_size = SkSize::Make(512, 512); // Fallback
        }
        svg_dom->setContainerSize(container_size);
        
        original_width = (int)container_size.width();
        original_height = (int)container_size.height();
        original_animation = false;
        original_format = "svg";
        
        SkBitmap bitmap;
        bitmap.allocPixels(SkImageInfo::MakeN32Premul((int)container_size.width(), (int)container_size.height()));
        SkCanvas canvas(bitmap);
        canvas.clear(SK_ColorTRANSPARENT);
        svg_dom->render(&canvas);
        
        frames.push_back({std::move(bitmap), 0});
        return true;
    }

    return false;
}

const uint8_t* ImageConverterInstance::encode(RenderFormat format, float quality, int speed, bool animation, int& out_size) {
    if (frames.empty()) {
        out_size = 0;
        return nullptr;
    }

    SkDynamicMemoryWStream stream;
    bool success = false;
    output_animation = false;
    output_format = "unknown";

    // Use the first frame for single-frame encoders
    const auto& first_bitmap = frames[0].bitmap;

    switch (format) {
        case RenderFormat::None: {
            if (raw_data) {
                out_size = (int)raw_data->size();
                const uint8_t* bytes = raw_data->bytes();
                context.set_last_output(raw_data);
                output_animation = original_animation;
                output_format = original_format;
                return bytes;
            }
            break;
        }
        case RenderFormat::PNG: {
            output_format = "png";
            SkPngEncoder::Options options;
            success = SkPngEncoder::Encode(&stream, first_bitmap.pixmap(), options);
            break;
        }
        case RenderFormat::WebP: {
            output_format = "webp";
            SkWebpEncoder::Options options;
            options.fQuality = (int)quality;
            if (quality >= 100.0f) {
                options.fCompression = SkWebpEncoder::Compression::kLossless;
            } else {
                options.fCompression = SkWebpEncoder::Compression::kLossy;
            }
            
            if (animation && frames.size() > 1) {
                output_animation = true;
                std::vector<SkEncoder::Frame> sk_frames;
                for (const auto& f : frames) {
                    sk_frames.push_back({f.bitmap.pixmap(), f.duration});
                }
                success = SkWebpEncoder::EncodeAnimated(&stream, SkSpan<const SkEncoder::Frame>(sk_frames.data(), sk_frames.size()), options);
            } else {
                success = SkWebpEncoder::Encode(&stream, first_bitmap.pixmap(), options);
            }
            break;
        }
        case RenderFormat::JPEG: {
            output_format = "jpeg";
            SkJpegEncoder::Options options;
            options.fQuality = (int)quality;
            success = SkJpegEncoder::Encode(&stream, first_bitmap.pixmap(), options);
            break;
        }
        case RenderFormat::SVG: {
            output_format = "svg";
            // Skia can export to SVG but it needs a canvas and drawing. 
            // For now, let's skip or return null.
            break;
        }
        case RenderFormat::AVIF: {
            output_format = "avif";
            avifRWData output = AVIF_DATA_EMPTY;
            avifImage* avif_image = avifImageCreate(first_bitmap.width(), first_bitmap.height(), 8, AVIF_PIXEL_FORMAT_YUV444);
            
            // Convert SkBitmap to AVIF RGB image
            avifRGBImage rgb;
            avifRGBImageSetDefaults(&rgb, avif_image);
            rgb.format = AVIF_RGB_FORMAT_RGBA;
            rgb.depth = 8;
            rgb.pixels = (uint8_t*)first_bitmap.getPixels();
            rgb.rowBytes = (uint32_t)first_bitmap.rowBytes();
            
            if (avifImageRGBToYUV(avif_image, &rgb) != AVIF_RESULT_OK) {
                avifImageDestroy(avif_image);
                break;
            }
            
            avifEncoder* encoder = avifEncoderCreate();
            encoder->quality = (int)quality;
            encoder->qualityAlpha = (int)quality;
            encoder->speed = speed; 
            
            avifResult result = avifEncoderWrite(encoder, avif_image, &output);
            if (result == AVIF_RESULT_OK) {
                stream.write(output.data, output.size);
                success = true;
            }
            
            avifRWDataFree(&output);
            avifEncoderDestroy(encoder);
            avifImageDestroy(avif_image);
            break;
        }
        case RenderFormat::PDF: {
            output_format = "pdf";
            // PDF output is complex for a single image, usually you'd use SkPDF.
            break;
        }
        case RenderFormat::RAW: {
            output_format = "raw";
            auto data = SkData::MakeWithCopy(first_bitmap.getPixels(), first_bitmap.computeByteSize());
            out_size = (int)data->size();
            const uint8_t* bytes = data->bytes();
            context.set_last_output(std::move(data));
            return bytes;
        }
        case RenderFormat::ThumbHash: {
            output_format = "thumbhash";
            std::vector<uint8_t> hash = rgbaToThumbHash(first_bitmap.width(), first_bitmap.height(), (const uint8_t*)first_bitmap.getPixels());
            auto data = SkData::MakeWithCopy(hash.data(), hash.size());
            out_size = (int)data->size();
            const uint8_t* bytes = data->bytes();
            context.set_last_output(std::move(data));
            return bytes;
        }
        default:
            break;
    }

    if (success) {
        auto data = stream.detachAsData();
        out_size = (int)data->size();
        const uint8_t* bytes = data->bytes();
        context.set_last_output(std::move(data));
        return bytes;
    }

    out_size = 0;
    return nullptr;
}

bool ImageConverterInstance::crop(int x, int y, int width, int height) {
    if (frames.empty()) return false;

    int original_w = frames[0].bitmap.width();
    int original_h = frames[0].bitmap.height();

    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x + width > original_w) width = original_w - x;
    if (y + height > original_h) height = original_h - y;

    if (width <= 0 || height <= 0) return false;

    for (auto& frame : frames) {
        SkImageInfo info = frame.bitmap.info().makeWH(width, height);
        SkBitmap cropped;
        cropped.allocPixels(info);

        SkCanvas canvas(cropped);
        canvas.clear(SK_ColorTRANSPARENT);

        SkRect src_rect = SkRect::MakeXYWH((float)x, (float)y, (float)width, (float)height);
        SkRect dest_rect = SkRect::MakeWH((float)width, (float)height);
        canvas.drawImageRect(frame.bitmap.asImage(), src_rect, dest_rect, SkSamplingOptions(), nullptr, SkCanvas::kStrict_SrcRectConstraint);

        frame.bitmap = cropped;
    }
    return true;
}

bool ImageConverterInstance::resize(int width, int height, FitMode fit) {
    if (frames.empty()) return false;

    int original_w = frames[0].bitmap.width();
    int original_h = frames[0].bitmap.height();

    if (width <= 0 && height <= 0) return true;
    if (width <= 0) width = (int)(original_w * ((float)height / original_h));
    if (height <= 0) height = (int)(original_h * ((float)width / original_w));

    float scale_x = (float)width / original_w;
    float scale_y = (float)height / original_h;

    int canvas_w = width;
    int canvas_h = height;
    float target_w = (float)width;
    float target_h = (float)height;
    float offset_x = 0;
    float offset_y = 0;

    if (fit == FitMode::Contain) {
        float scale = std::min(scale_x, scale_y);
        target_w = original_w * scale;
        target_h = original_h * scale;
        canvas_w = (int)target_w;
        canvas_h = (int)target_h;
    } else if (fit == FitMode::Cover) {
        float scale = std::max(scale_x, scale_y);
        target_w = original_w * scale;
        target_h = original_h * scale;
        offset_x = ((float)width - target_w) / 2.0f;
        offset_y = ((float)height - target_h) / 2.0f;
    }

    for (auto& frame : frames) {
        SkImageInfo info = frame.bitmap.info().makeWH(canvas_w, canvas_h);
        SkBitmap resized;
        resized.allocPixels(info);

        SkCanvas canvas(resized);
        canvas.clear(SK_ColorTRANSPARENT);
        
        SkRect dest_rect = SkRect::MakeXYWH(offset_x, offset_y, target_w, target_h);
        canvas.drawImageRect(frame.bitmap.asImage(), 
                             dest_rect, 
                             SkSamplingOptions(SkFilterMode::kLinear));

        frame.bitmap = resized;
    }
    return true;
}

int ImageConverterInstance::get_original_width() const { return original_width; }
int ImageConverterInstance::get_original_height() const { return original_height; }
bool ImageConverterInstance::is_original_animation() const { return original_animation; }
std::string ImageConverterInstance::get_original_format() const { return original_format; }
int ImageConverterInstance::get_width() const { return frames.empty() ? 0 : frames[0].bitmap.width(); }
int ImageConverterInstance::get_height() const { return frames.empty() ? 0 : frames[0].bitmap.height(); }
bool ImageConverterInstance::is_animation() const { return output_animation; }
std::string ImageConverterInstance::get_format() const { return output_format; }

// --- API Functions (Wrappers) ---

ImageConverterInstance* api_create_instance() { return new ImageConverterInstance(); }

void api_destroy_instance(ImageConverterInstance* inst) { delete inst; }

static sk_sp<SkData> patch_svg_data(const sk_sp<SkData>& data) {
    std::string_view svg((const char*)data->data(), data->size());
    std::string patched_svg;
    bool changed = false;

    // 1. Patch feDropShadow
    {
        std::string result;
        std::string_view searchRange = svg;
        bool step_changed = false;

        while (auto m = ctre::search<"<feDropShadow([^>]*?)/?>">(searchRange)) {
            step_changed = true;
            changed = true;
            result.append(searchRange.substr(0, m.get<0>().begin() - searchRange.begin()));
            
            std::string_view attrs = m.get<1>().to_view();
            std::string dx = "0", dy = "0", stdDev = "0", floodColor = "black", floodOpacity = "1", in = "SourceGraphic";
            
            std::string_view attrSearchRange = attrs;
            while (auto am = ctre::search<"([\\w\\-]+)\\s*=\\s*\"([^\"]*)\"">(attrSearchRange)) {
                std::string_view name = am.get<1>().to_view();
                std::string_view value = am.get<2>().to_view();
                if (name == "dx") dx = std::string(value);
                else if (name == "dy") dy = std::string(value);
                else if (name == "stdDeviation") stdDev = std::string(value);
                else if (name == "flood-color") floodColor = std::string(value);
                else if (name == "flood-opacity") floodOpacity = std::string(value);
                else if (name == "in") in = std::string(value);
                attrSearchRange = attrSearchRange.substr(am.get<0>().end() - attrSearchRange.begin());
            }

            char buf[2048];
            snprintf(buf, sizeof(buf),
                "<feGaussianBlur in=\"%s\" stdDeviation=\"%s\" result=\"_dropShadowBlur\"/>"
                "<feOffset in=\"_dropShadowBlur\" dx=\"%s\" dy=\"%s\" result=\"_dropShadowOffset\"/>"
                "<feFlood flood-color=\"%s\" flood-opacity=\"%s\" result=\"_dropShadowFlood\"/>"
                "<feComposite in=\"_dropShadowFlood\" in2=\"_dropShadowOffset\" operator=\"in\" result=\"_dropShadow\"/>"
                "<feMerge><feMergeNode in=\"_dropShadow\"/><feMergeNode in=\"%s\"/></feMerge>",
                in.c_str(), stdDev.c_str(), dx.c_str(), dy.c_str(), floodColor.c_str(), floodOpacity.c_str(), in.c_str());
            
            result.append(buf);
            searchRange = searchRange.substr(m.get<0>().end() - searchRange.begin());
        }
        if (step_changed) {
            result.append(searchRange);
            patched_svg = std::move(result);
            svg = patched_svg;
        }
    }

    // 2. Patch pattern containing linearGradient (Satori workaround)
    {
        std::string result;
        std::string_view searchRange = svg;
        bool patternChanged = false;

        std::vector<std::pair<std::string, std::string>> replacements;

        while (auto m = ctre::search<"<pattern\\s+([^>]*?id=\"([^\"]+)\"[^>]*?)>(.*?)</pattern>">(searchRange)) {
            std::string_view patternAttrs = m.get<1>().to_view();
            std::string_view patternId = m.get<2>().to_view();
            std::string_view content = m.get<3>().to_view();

            if (patternAttrs.find("patternUnits=\"objectBoundingBox\"") != std::string_view::npos) {
                if (auto gm = ctre::search<"<linearGradient\\s+([^>]*?id=\"([^\"]+)\"[^>]*?)>(.*?)</linearGradient>">(content)) {
                    patternChanged = true;
                    changed = true;
                    result.append(searchRange.substr(0, m.get<0>().begin() - searchRange.begin()));

                    std::string_view gradAttrsWithId = gm.get<1>().to_view();
                    std::string_view gradId = gm.get<2>().to_view();
                    std::string_view gradContent = gm.get<3>().to_view();

                    // Extract attributes without the ID to avoid duplicates
                    std::string gradAttrs;
                    std::string_view gaRange = gradAttrsWithId;
                    while (auto gam = ctre::search<"id=\"[^\"]+\"">(gaRange)) {
                        gradAttrs.append(gaRange.substr(0, gam.get<0>().begin() - gaRange.begin()));
                        gaRange = gaRange.substr(gam.get<0>().end() - gaRange.begin());
                    }
                    gradAttrs.append(gaRange);

                    // Put the linearGradient in place of the pattern
                    result.append("<linearGradient id=\"");
                    result.append(std::string(gradId));
                    result.append("\" ");
                    result.append(gradAttrs);
                    result.append(">");
                    result.append(std::string(gradContent));
                    result.append("</linearGradient>");

                    // Replace all references to this pattern with references to the nested gradient
                    replacements.push_back({"url(#" + std::string(patternId) + ")", "url(#" + std::string(gradId) + ")"});
                    
                    searchRange = searchRange.substr(m.get<0>().end() - searchRange.begin());
                    continue;
                }
            }
            result.append(searchRange.substr(0, m.get<0>().end() - searchRange.begin()));
            searchRange = searchRange.substr(m.get<0>().end() - searchRange.begin());
        }

        if (patternChanged) {
            result.append(searchRange);
            patched_svg = std::move(result);
            
            // Apply reference replacements on the full patched string
            for (const auto& r : replacements) {
                size_t pos = 0;
                while ((pos = patched_svg.find(r.first, pos)) != std::string::npos) {
                    patched_svg.replace(pos, r.first.length(), r.second);
                    pos += r.second.length();
                }
            }
            svg = patched_svg;
        }
    }

    // 3. Patch href to xlink:href for embedded images and add xlink namespace if needed
    {
        std::string result;
        std::string_view searchRange = svg;
        bool step_changed = false;
        while (auto m = ctre::search<"(<image[^>]*?)\\s+href=\"data:([^/]+/[^;]+;base64,[^\"]+)\"">(searchRange)) {
            step_changed = true;
            changed = true;
            result.append(searchRange.substr(0, m.get<0>().begin() - searchRange.begin()));
            result.append(m.get<1>().to_view());
            result.append(" xlink:href=\"data:");
            result.append(m.get<2>().to_view());
            result.append("\"");
            searchRange = searchRange.substr(m.get<0>().end() - searchRange.begin());
        }
        if (step_changed) {
            result.append(searchRange);
            patched_svg = std::move(result);
            svg = patched_svg;
            
            // Ensure xmlns:xlink is present
            if (svg.find("xmlns:xlink") == std::string::npos) {
                size_t pos = svg.find("<svg");
                if (pos != std::string::npos) {
                    size_t end_pos = svg.find(">", pos);
                    if (end_pos != std::string::npos) {
                        patched_svg.insert(end_pos, " xmlns:xlink=\"http://www.w3.org/1999/xlink\"");
                        svg = patched_svg;
                    }
                }
            }
        }
    }

    if (!changed) return data;
    return SkData::MakeWithCopy(svg.data(), svg.size());
}

const uint8_t* api_load_image(ImageConverterInstance* inst, const uint8_t* data, size_t size) {
    inst->raw_data = SkData::MakeWithCopy(data, size);
    sk_sp<SkData> sk_data = inst->raw_data;
    if (size >= 4 && data[0] == '<') {
        sk_data = patch_svg_data(sk_data);
    }
    if (inst->load_image((const uint8_t*)sk_data->data(), sk_data->size())) {
        return (const uint8_t*)1; // Success indicator
    }
    return nullptr;
}

const uint8_t* api_encode(ImageConverterInstance* inst, int format, float quality, int speed, bool animation, int& out_size) {
    return inst->encode((RenderFormat)format, quality, speed, animation, out_size);
}

bool api_crop(ImageConverterInstance* inst, int x, int y, int width, int height) {
    return inst->crop(x, y, width, height);
}

bool api_resize(ImageConverterInstance* inst, int width, int height, int fit) {
    return inst->resize(width, height, (FitMode)fit);
}

int api_get_original_width(ImageConverterInstance* inst) { return inst->get_original_width(); }
int api_get_original_height(ImageConverterInstance* inst) { return inst->get_original_height(); }
bool api_is_original_animation(ImageConverterInstance* inst) { return inst->is_original_animation(); }
std::string api_get_original_format(ImageConverterInstance* inst) { return inst->get_original_format(); }
int api_get_width(ImageConverterInstance* inst) { return inst->get_width(); }
int api_get_height(ImageConverterInstance* inst) { return inst->get_height(); }
bool api_is_animation(ImageConverterInstance* inst) { return inst->is_animation(); }
std::string api_get_format(ImageConverterInstance* inst) { return inst->get_format(); }

void api_set_log_level(int level) { g_log_level = level; }
