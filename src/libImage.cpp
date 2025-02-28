#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <webp/encode.h>
#include <SDL2/SDL2_rotozoom.h>
#include <SDL2/SDL_image.h>
#include <SDL3/SDL.h>
#include <libexif/exif-data.h>
#include <avif/avif.h>
#include <lunasvg.h>

using namespace emscripten;
using namespace lunasvg;

EM_JS(void, js_console_log, (const char *str), {
    console.log(UTF8ToString(str));
});

class MemoryManager
{
private:
    uint8_t *m_ptr;

public:
    MemoryManager()
    {
        m_ptr = NULL;
    }
    uint8_t *allocate(const uint8_t *data, size_t size)
    {
        uint8_t *ptr = new uint8_t[size];
        memcpy(ptr, data, size);
        m_ptr = ptr;
        return ptr;
    }
    void release()
    {
        if (m_ptr)
        {
            delete m_ptr;
        }
    }
};

MemoryManager memoryManager;

class MemoryRW
{
public:
    MemoryRW()
    {
        m_rw = SDL_AllocRW();
        m_rw->hidden.unknown.data1 = &m_buffer;
        m_rw->write = MemWrite;
        m_rw->close = MemClose;
    }
    ~MemoryRW()
    {
        SDL_FreeRW(m_rw);
    }
    operator SDL_RWops *() const { return m_rw; }
    size_t size() const { return m_buffer.size(); }
    const uint8_t *data() const { return m_buffer.data(); }

protected:
    static size_t MemWrite(SDL_RWops *context, const void *ptr, size_t size, size_t num)
    {
        std::vector<uint8_t> *buffer = (std::vector<uint8_t> *)context->hidden.unknown.data1;
        const uint8_t *bytes = (const uint8_t *)ptr;
        buffer->insert(buffer->end(), bytes, bytes + size * num);
        return num;
    }
    static int MemClose(SDL_RWops *context)
    {
        return 0;
    }

private:
    SDL_RWops *m_rw;
    std::vector<uint8_t> m_buffer;
};

int getOrientation(std::string img)
{
    int orientation = 1;
    ExifData *ed = exif_data_new_from_data((const unsigned char *)img.c_str(), img.size());
    if (!ed)
    {
        return orientation;
    }
    ExifEntry *entry = exif_content_get_entry(ed->ifd[EXIF_IFD_0], EXIF_TAG_ORIENTATION);
    if (entry)
    {
        orientation = exif_get_short(entry->data, exif_data_get_byte_order(entry->parent->parent));
    }
    exif_data_unref(ed);
    return orientation;
}

val createResult(size_t size, const uint8_t *data, float originalWidth, float originalHeight, float width, float height)
{
    uint8_t *ptr = memoryManager.allocate(data, size);
    val result = val::object();
    result.set("data", val(typed_memory_view(size, ptr)));
    result.set("originalWidth", originalWidth);
    result.set("originalHeight", originalHeight);
    result.set("width", width);
    result.set("height", height);
    return result;
}

void releaseResult()
{
    memoryManager.release();
}

val optimize(std::string img_in, float width, float height, float quality, std::string format, int speed = 6)
{
    int orientation = getOrientation(img_in);
    Bitmap bitmap;
    SDL_RWops *rw = SDL_RWFromConstMem(img_in.c_str(), img_in.size());
    if (!rw)
    {
        return val::null();
    }
    SDL_Surface *srcSurface;
    if (IMG_isSVG(rw))
    {
        auto document = Document::loadFromData(img_in.c_str(), img_in.length());
        if (!document.get())
            return val::null();
        bitmap = document->renderToBitmap(width == 0 ? -1 : width, height == 0 ? -1 : height);
        bitmap.convertToRGBA();
        if (bitmap.isNull())
            return val::null();
        srcSurface = SDL_CreateRGBSurfaceWithFormatFrom(bitmap.data(), bitmap.width(), bitmap.height(), 32, bitmap.stride(), SDL_PIXELFORMAT_RGBA32);
    }
    else
    {
        srcSurface = IMG_Load_RW(rw, 1);
    }
    SDL_FreeRW(rw);
    if (!srcSurface)
    {
        return val::null();
    }

    int srcWidth = srcSurface->w;
    int srcHeight = srcSurface->h;
    if (srcWidth == 0 || srcHeight == 0)
    {
        SDL_FreeSurface(srcSurface);
        return val::null();
    }
    if (format == "none")
    {
        val result = createResult(img_in.size(), (const uint8_t *)img_in.c_str(), srcWidth, srcHeight, srcWidth, srcHeight);
        return result;
    }

    int outWidth = width ? width : srcWidth;
    int outHeight = height ? height : srcHeight;
    float aspectSrc = static_cast<float>(srcWidth) / srcHeight;
    float aspectDest = outWidth / outHeight;

    if (aspectSrc > aspectDest)
    {
        outHeight = outWidth / aspectSrc;
    }
    else
    {
        outWidth = outHeight * aspectSrc;
    }

    SDL_Surface *newSurface = zoomSurface(srcSurface, (float)outWidth / srcWidth, (float)outHeight / srcHeight, SMOOTHING_ON);
    SDL_FreeSurface(srcSurface);
    if (!newSurface)
    {
        return val::null();
    }

    if (orientation > 1)
    {
        double angle = 0;
        double x = 1;
        double y = 1;
        switch (orientation)
        {
        case 2:
            x = -1.0;
            break;
        case 3:
            angle = 180.0;
            break;
        case 4:
            y = -1.0;
            break;
        case 5:
            angle = 90.0;
            x = -1.0;
            break;
        case 6:
            angle = 270.0;
            break;
        case 7:
            angle = 270.0;
            x = -1.0;
            break;
        case 8:
            angle = 90.0;
            break;
        }
        SDL_Surface *rotatedSurface = rotozoomSurfaceXY(newSurface, angle, x, y, SMOOTHING_ON);
        SDL_FreeSurface(newSurface);
        newSurface = rotatedSurface;
    }

    if (format == "png" || format == "jpeg")
    {
        MemoryRW memoryRW;
        if (format == "png")
        {
            IMG_SavePNG_RW(newSurface, memoryRW, 1);
        }
        else
        {
            IMG_SaveJPG_RW(newSurface, memoryRW, 1, quality);
        }
        SDL_FreeSurface(newSurface);
        val result = val::null();
        if (memoryRW.size())
        {
            result = createResult(memoryRW.size(), memoryRW.data(), srcWidth, srcHeight, outWidth, outHeight);
        }
        return result;
    }
    else
    {
        if (newSurface->format->format != SDL_PIXELFORMAT_RGBA32)
        {
            SDL_Surface *convertedSurface = SDL_ConvertSurfaceFormat(newSurface, SDL_PIXELFORMAT_RGBA32, 0);
            SDL_FreeSurface(newSurface);
            if (convertedSurface == NULL)
            {
                return val::null();
            }
            newSurface = convertedSurface;
        }
        if (format == "webp")
        {
            uint8_t *img_out;
            val result = val::null();
            int width = newSurface->w;
            int height = newSurface->h;
            int stride = width * 4;
            size_t size = WebPEncodeRGBA(reinterpret_cast<uint8_t *>(newSurface->pixels), width, height, stride, quality, &img_out);
            if (size > 0 && img_out)
            {
                result = createResult(size, img_out, srcWidth, srcHeight, outWidth, outHeight);
            }
            WebPFree(img_out);
            SDL_FreeSurface(newSurface);
            return result;
        }
        else
        {
            int width = newSurface->w;
            int height = newSurface->h;
            avifImage *image = avifImageCreate(width, height, 8, AVIF_PIXEL_FORMAT_YUV444);

            avifRGBImage rgb;
            avifRGBImageSetDefaults(&rgb, image);
            rgb.depth = 8;
            rgb.format = AVIF_RGB_FORMAT_RGBA;
            rgb.pixels = (uint8_t *)newSurface->pixels;
            rgb.rowBytes = width * 4;

            if (avifImageRGBToYUV(image, &rgb) != AVIF_RESULT_OK)
            {
                return val::null();
            }
            avifEncoder *encoder = avifEncoderCreate();
            encoder->quality = (int)((quality) / 100 * 63);
            encoder->speed = speed;

            avifRWData raw = AVIF_DATA_EMPTY;

            avifResult encodeResult = avifEncoderWrite(encoder, image, &raw);
            avifEncoderDestroy(encoder);
            avifImageDestroy(image);
            if (encodeResult != AVIF_RESULT_OK)
            {
                return val::null();
            }
            val result = createResult(raw.size, raw.data, srcWidth, srcHeight, outWidth, outHeight);
            avifRWDataFree(&raw);
            return result;
        }
    }
}

EMSCRIPTEN_BINDINGS(my_module)
{
    function("optimize", &optimize);
    function("releaseResult", &releaseResult);
}
