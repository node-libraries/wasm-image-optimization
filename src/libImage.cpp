#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <webp/encode.h>
#include <SDL_image.h>
#include <SDL2/SDL.h>

using namespace emscripten;

val optimize(std::string img_in, float width, float height, float quality) {
    SDL_RWops* rw = SDL_RWFromConstMem(img_in.c_str(), img_in.size());
    if (!rw) {
        return val::null();
    }
    
    SDL_Surface* srcSurface = IMG_Load_RW(rw, 1);
    SDL_FreeRW(rw);
    if (!srcSurface) {
        return val::null();
    }
    
    int srcWidth = srcSurface->w;
    int srcHeight = srcSurface->h;
    if (srcWidth == 0 || srcHeight == 0) {
        SDL_FreeSurface(srcSurface);
        return val::null();
    }

    int outWidth = width?width:srcWidth;
    int outHeight = height?height:srcHeight;
    float aspectSrc = static_cast<float>(srcWidth) / srcHeight;
    float aspectDest = outWidth / outHeight;
    
    if (aspectSrc > aspectDest) {
        outHeight = outWidth / aspectSrc;
    } else {
        outWidth = outHeight * aspectSrc;
    }

    SDL_Surface* newSurface = SDL_CreateRGBSurfaceWithFormat(0, static_cast<int>(outWidth), static_cast<int>(outHeight), 32, SDL_PIXELFORMAT_RGBA32);
    if (!newSurface) {
        SDL_FreeSurface(srcSurface);
        return val::null();
    }

    SDL_BlitScaled(srcSurface, nullptr, newSurface, nullptr);
    SDL_FreeSurface(srcSurface);

    uint8_t* img_out = nullptr;
    int stride = static_cast<int>(outWidth) * 4;
    size_t size = WebPEncodeRGBA(reinterpret_cast<uint8_t*>(newSurface->pixels), static_cast<int>(outWidth), static_cast<int>(outHeight), stride, quality, &img_out);

    if (size == 0 || !img_out) {
        SDL_FreeSurface(newSurface);
        return val::null();
    }
    
    val result = val::global("Uint8Array").new_(typed_memory_view(size, img_out));
    WebPFree(img_out);
    SDL_FreeSurface(newSurface);

    return result;
}

EMSCRIPTEN_BINDINGS(my_module) {
  function("optimize", &optimize);
}
