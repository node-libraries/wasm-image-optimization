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
#include <regex>

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
        if (g_log_level >= 3) {
            char buf[128];
            snprintf(buf, sizeof(buf), "Codec origin: %d, swap: %d", (int)origin, swapDimensions);
            image_converter_log(LogLevel::Info, buf);
        }

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
        nullptr, skresources::ImageDecodeStrategy::kPreDecode);
    auto svg_dom = SkSVGDOM::Builder()
                       .setResourceProvider(std::move(resource_provider))
                       .make(stream);
    if (svg_dom) {
        auto container_size = svg_dom->containerSize();
        if (container_size.isEmpty()) {
            container_size = SkSize::Make(512, 512); // Fallback
        }
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
    std::string svg((const char*)data->data(), data->size());
    bool changed = false;

    // 1. Patch feDropShadow
    {
        std::regex re("<feDropShadow([^>]*?)/?>");
        std::smatch m;
        std::string result;
        std::string::const_iterator searchStart(svg.cbegin());

        while (std::regex_search(searchStart, svg.cend(), m, re)) {
            changed = true;
            result.append(searchStart, m[0].first);
            
            std::string attrs = m[1].str();
            std::string dx = "0", dy = "0", stdDev = "0", floodColor = "black", floodOpacity = "1", in = "SourceGraphic";
            
            std::regex attr_re("([\\w-]+)\\s*=\\s*\"([^\"]*)\"");
            std::smatch am;
            std::string::const_iterator attrStart(attrs.cbegin());
            while (std::regex_search(attrStart, attrs.cend(), am, attr_re)) {
                std::string name = am[1].str();
                std::string value = am[2].str();
                if (name == "dx") dx = value;
                else if (name == "dy") dy = value;
                else if (name == "stdDeviation") stdDev = value;
                else if (name == "flood-color") floodColor = value;
                else if (name == "flood-opacity") floodOpacity = value;
                else if (name == "in") in = value;
                attrStart = am.suffix().first;
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
            searchStart = m.suffix().first;
        }
        if (changed) {
            result.append(searchStart, svg.cend());
            svg = result;
        }
    }

    // 2. Patch pattern containing linearGradient (Satori workaround)
    {
        // More robust pattern matching that doesn't rely on specific attribute order
        std::regex pat_re("<pattern\\s+([^>]*?id=\"([^\"]+)\"[^>]*?)>(.*?)</pattern>");
        std::smatch m;
        std::string result;
        std::string::const_iterator searchStart(svg.cbegin());
        bool patternChanged = false;

        while (std::regex_search(searchStart, svg.cend(), m, pat_re)) {
            std::string patternAttrs = m[1].str();
            std::string patternId = m[2].str();
            std::string content = m[3].str();

            if (patternAttrs.find("patternUnits=\"objectBoundingBox\"") != std::string::npos) {
                std::regex grad_re("<linearGradient\\s+([^>]*?id=\"([^\"]+)\"[^>]*?)>(.*?)</linearGradient>");
                std::smatch gm;
                if (std::regex_search(content.cbegin(), content.cend(), gm, grad_re)) {
                    patternChanged = true;
                    changed = true;
                    result.append(searchStart, m[0].first);

                    std::string gradAttrsWithId = gm[1].str();
                    std::string gradId = gm[2].str();
                    std::string gradContent = gm[3].str();

                    // Extract attributes without the ID to avoid duplicates
                    std::string gradAttrs = std::regex_replace(gradAttrsWithId, std::regex("id=\"[^\"]+\""), "");

                    // Put the linearGradient in place of the pattern
                    result.append("<linearGradient id=\"");
                    result.append(gradId);
                    result.append("\" ");
                    result.append(gradAttrs);
                    result.append(">");
                    result.append(gradContent);
                    result.append("</linearGradient>");

                    // Replace all references to this pattern with references to the nested gradient
                    std::string ref = "url(#" + patternId + ")";
                    std::string rep = "url(#" + gradId + ")";
                    size_t pos = 0;
                    while ((pos = svg.find(ref, pos)) != std::string::npos) {
                        svg.replace(pos, ref.length(), rep);
                        pos += rep.length();
                    }
                    searchStart = m.suffix().first;
                    continue;
                }
            }
            result.append(searchStart, m.suffix().first);
            searchStart = m.suffix().first;
        }

        if (patternChanged) {
            result.append(searchStart, svg.cend());
            svg = result;
        }
    }

    if (g_log_level >= 4) {
        image_converter_log(LogLevel::Debug, "Patched SVG:");
        image_converter_log(LogLevel::Debug, svg.c_str());
    }

    if (!changed) return data;
    return SkData::MakeWithCopy(svg.c_str(), svg.size());
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
