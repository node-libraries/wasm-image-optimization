#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <webp/encode.h>
#include <SDL_image.h>
#include <SDL2/SDL.h>

using namespace emscripten;

EM_JS(void, js_console_log, (const char* str), {
  console.log(UTF8ToString(str));
});

struct MemBuffer {
    std::vector<uint8_t> buffer;
};

size_t MemWrite(SDL_RWops* context, const void* ptr, size_t size, size_t num) {
    MemBuffer* mem = (MemBuffer*)context->hidden.unknown.data1;
    const uint8_t* bytes = (const uint8_t*)ptr;
    mem->buffer.insert(mem->buffer.end(), bytes, bytes + size * num);
    return num;
}
int MemClose(SDL_RWops* context) {
    return 0;
}


SDL_RWops* CreateMemBuffer() {
    SDL_RWops* rw = SDL_AllocRW();
    if (rw != nullptr) {
        rw->hidden.unknown.data1 = new MemBuffer();
        rw->write = MemWrite;
        rw->close = MemClose;
    }
    return rw;
}

void FreeMemBuffer(SDL_RWops* rw) {
    if (rw != nullptr) {
        delete (MemBuffer*)rw->hidden.unknown.data1;
        SDL_FreeRW(rw);
    }
}


val optimize(std::string img_in, float width, float height, float quality,std::string format) {
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
    size_t size = 0;
    if(format == "png" || format == "jpeg"){
        SDL_RWops* rw = CreateMemBuffer();
        if(format == "png"){
            IMG_SavePNG_RW(newSurface, rw, 1);
        } else {
            IMG_SaveJPG_RW(newSurface, rw, 1,quality);
        }
        SDL_FreeSurface(newSurface);
        MemBuffer* mem = (MemBuffer*)rw->hidden.unknown.data1;
        size = mem->buffer.size();
        val result = val::null();
        if(size){
            result = val::global("Uint8Array").new_(typed_memory_view(size,  mem->buffer.data()));
        }
        FreeMemBuffer(rw);
        return result;
    } else {
        val result = val::null();
        int stride = static_cast<int>(outWidth) * 4;
        size = WebPEncodeRGBA(reinterpret_cast<uint8_t*>(newSurface->pixels), static_cast<int>(outWidth), static_cast<int>(outHeight), stride, quality, &img_out);
        if (size > 0 && img_out) {
            result = val::global("Uint8Array").new_(typed_memory_view(size, img_out));
        }
        WebPFree(img_out);
        SDL_FreeSurface(newSurface);
        return result;
    }

}

EMSCRIPTEN_BINDINGS(my_module) {
  function("optimize", &optimize);
}
