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

class Image
{
private:
    float m_width;
    float m_height;
    int m_orientation;
    SDL_RWops *m_rw;
    std::string *m_img;
    float m_srcWidth;
    float m_srcHeight;
    lunasvg::Bitmap m_bitmap;

public:
    Image(std::string &img, float width, float height)
    {
        m_width = width;
        m_height = height;
        m_orientation = getOrientation(img.c_str(), img.size());
        m_rw = SDL_RWFromConstMem(img.c_str(), img.size());
        m_img = &img;
    }
    ~Image()
    {
        if (m_rw)
            SDL_FreeRW(m_rw);
    }
    int getOrientation(const char *data, size_t size)
    {
        int orientation = 1;
        ExifData *ed = exif_data_new_from_data((const unsigned char *)data, size);
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

    SDL_Surface *getSurface(bool convert)
    {
        if (!m_rw)
        {
            return nullptr;
        }
        SDL_Surface *srcSurface;
        if (IMG_isSVG(m_rw))
        {
            auto document = Document::loadFromData(m_img->c_str(), m_img->length());
            if (!document.get())
                return nullptr;
            m_bitmap = document->renderToBitmap(m_width ? m_width : -1, m_height ? m_height : -1);
            m_bitmap.convertToRGBA();
            if (m_bitmap.isNull())
                return nullptr;
            srcSurface = SDL_CreateRGBSurfaceWithFormatFrom(m_bitmap.data(), m_bitmap.width(), m_bitmap.height(), 32, m_bitmap.stride(), SDL_PIXELFORMAT_RGBA32);
        }
        else
        {
            srcSurface = IMG_Load_RW(m_rw, 1);
        }
        m_srcWidth = srcSurface->w;
        m_srcHeight = srcSurface->h;
        if (!convert)
            return srcSurface;

        if (m_srcWidth == 0 || m_srcHeight == 0)
        {
            SDL_FreeSurface(srcSurface);
            return nullptr;
        }

        int outWidth = m_width ? m_width : m_srcWidth;
        int outHeight = m_height ? m_height : m_srcHeight;
        float aspectSrc = static_cast<float>(m_srcWidth) / m_srcHeight;
        float aspectDest = outWidth / outHeight;

        if (aspectSrc > aspectDest)
        {
            outHeight = outWidth / aspectSrc;
        }
        else
        {
            outWidth = outHeight * aspectSrc;
        }

        SDL_Surface *newSurface = zoomSurface(srcSurface, (float)outWidth / m_srcWidth, (float)outHeight / m_srcHeight, SMOOTHING_ON);
        SDL_FreeSurface(srcSurface);
        if (!newSurface)
        {
            return nullptr;
        }

        if (m_orientation > 1)
        {
            double angle = 0;
            double x = 1;
            double y = 1;
            switch (m_orientation)
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
        return newSurface;
    }
    float getSrcWidth() const
    {
        return m_srcWidth;
    }
    float getSrcHeight() const
    {
        return m_srcHeight;
    }
};

val optimize(std::string img_in, float width, float height, float quality, std::string format, int speed = 6)
{
    Image image(img_in, width, height);
    auto surface = image.getSurface(format != "none");
    if (!surface)
    {
        return val::null();
    }
    if (format == "none")
    {
        val result = createResult(img_in.size(), (const uint8_t *)img_in.c_str(), surface->w, surface->h, surface->w, surface->h);
        SDL_FreeSurface(surface);
        return result;
    }

    if (format == "png" || format == "jpeg")
    {
        MemoryRW memoryRW;
        if (format == "png")
        {
            IMG_SavePNG_RW(surface, memoryRW, 1);
        }
        else
        {
            IMG_SaveJPG_RW(surface, memoryRW, 1, quality);
        }
        SDL_FreeSurface(surface);
        val result = val::null();
        if (memoryRW.size())
        {
            result = createResult(memoryRW.size(), memoryRW.data(), image.getSrcWidth(), image.getSrcHeight(), surface->w, surface->h);
        }
        return result;
    }
    else
    {
        if (surface->format->format != SDL_PIXELFORMAT_RGBA32)
        {
            SDL_Surface *convertedSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
            SDL_FreeSurface(surface);
            if (convertedSurface == NULL)
            {
                return val::null();
            }
            surface = convertedSurface;
        }
        if (format == "webp")
        {
            uint8_t *img_out;
            val result = val::null();
            int width = surface->w;
            int height = surface->h;
            int stride = width * 4;

            size_t size = WebPEncodeRGBA(reinterpret_cast<uint8_t *>(surface->pixels), width, height, stride, quality, &img_out);
            if (size > 0 && img_out)
            {
                result = createResult(size, img_out, image.getSrcWidth(), image.getSrcHeight(), surface->w, surface->h);
            }
            WebPFree(img_out);
            SDL_FreeSurface(surface);
            return result;
        }
        else
        {
            int width = surface->w;
            int height = surface->h;
            avifImage *avifImage = avifImageCreate(width, height, 8, AVIF_PIXEL_FORMAT_YUV444);

            avifRGBImage rgb;
            avifRGBImageSetDefaults(&rgb, avifImage);
            rgb.depth = 8;
            rgb.format = AVIF_RGB_FORMAT_RGBA;
            rgb.pixels = (uint8_t *)surface->pixels;
            rgb.rowBytes = width * 4;

            if (avifImageRGBToYUV(avifImage, &rgb) != AVIF_RESULT_OK)
            {
                return val::null();
            }
            avifEncoder *encoder = avifEncoderCreate();
            encoder->quality = (int)((quality) / 100 * 63);
            encoder->speed = speed;

            avifRWData raw = AVIF_DATA_EMPTY;

            avifResult encodeResult = avifEncoderWrite(encoder, avifImage, &raw);
            avifEncoderDestroy(encoder);
            avifImageDestroy(avifImage);
            if (encodeResult != AVIF_RESULT_OK)
            {
                return val::null();
            }
            val result = createResult(raw.size, raw.data, image.getSrcWidth(), image.getSrcHeight(), surface->w, surface->h);
            SDL_FreeSurface(surface);
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
