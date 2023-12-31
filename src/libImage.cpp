#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <webp/encode.h>
#include <SDL_image.h>
#include <SDL2/SDL.h>

using namespace emscripten;

EM_JS(void, js_console_log, (const char *str), {
    console.log(UTF8ToString(str));
});

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

val optimize(std::string img_in, float width, float height, float quality, std::string format)
{
    SDL_RWops *rw = SDL_RWFromConstMem(img_in.c_str(), img_in.size());
    if (!rw)
    {
        return val::null();
    }

    SDL_Surface *srcSurface = IMG_Load_RW(rw, 1);
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

    SDL_Surface *newSurface = SDL_CreateRGBSurfaceWithFormat(0, static_cast<int>(outWidth), static_cast<int>(outHeight), 32, SDL_PIXELFORMAT_RGBA32);
    if (!newSurface)
    {
        SDL_FreeSurface(srcSurface);
        return val::null();
    }

    SDL_BlitScaled(srcSurface, nullptr, newSurface, nullptr);
    SDL_FreeSurface(srcSurface);

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
            result = val::global("Uint8Array").new_(typed_memory_view(memoryRW.size(), memoryRW.data()));
        }
        return result;
    }
    else
    {
        uint8_t *img_out;
        val result = val::null();
        int stride = static_cast<int>(outWidth) * 4;
        size_t size = WebPEncodeRGBA(reinterpret_cast<uint8_t *>(newSurface->pixels), static_cast<int>(outWidth), static_cast<int>(outHeight), stride, quality, &img_out);
        if (size > 0 && img_out)
        {
            result = val::global("Uint8Array").new_(typed_memory_view(size, img_out));
        }
        WebPFree(img_out);
        SDL_FreeSurface(newSurface);
        return result;
    }
}

EMSCRIPTEN_BINDINGS(my_module)
{
    function("optimize", &optimize);
}
