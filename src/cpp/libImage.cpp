#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <SDL2/SDL_image.h>
#include <SDL3/SDL.h>
#include <avif/avif.h>
#include <webp/encode.h>
#include "libs/MemoryManager.h"
#include "libs/MemoryRW.h"
#include "libs/Image.h"

using namespace emscripten;

EM_JS(void, js_console_log, (const char *str), {
  console.log(UTF8ToString(str));
});

MemoryManager memoryManager;

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

val optimize(std::string img_in, float width, float height, float quality, std::string format, int speed = 6, bool filter = true)
{
  Image image(img_in, width, height);
  Surface surface = image.getSurface(format != "none", filter);
  if (!surface)
  {
    return val::null();
  }
  if (format == "none")
  {
    val result = createResult(img_in.size(), (const uint8_t *)img_in.c_str(), surface->w, surface->h, surface->w, surface->h);
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
    int width = surface->w;
    int height = surface->h;
    surface.release();
    val result = val::null();
    if (memoryRW.size())
    {
      result = createResult(memoryRW.size(), memoryRW.data(), image.getSrcWidth(), image.getSrcHeight(), width, height);
    }
    return result;
  }
  else
  {
    if (surface->format->format != SDL_PIXELFORMAT_RGBA32)
    {
      Surface convertedSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
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
        avifImageDestroy(avifImage);
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
        avifRWDataFree(&raw);
        return val::null();
      }
      val result = createResult(raw.size, raw.data, image.getSrcWidth(), image.getSrcHeight(), width, height);
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
