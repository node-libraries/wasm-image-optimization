#include <SDL2/SDL2_rotozoom.h>
#include <libexif/exif-data.h>
#include <lancir.h>
#include "Image.h"

using namespace lunasvg;

Image::Image(std::string &img, float width, float height)
{
  m_width = width;
  m_height = height;
  m_orientation = getOrientation(img.c_str(), img.size());
  m_rw = SDL_RWFromConstMem(img.c_str(), img.size());
  m_img = &img;
}
Image::~Image()
{
  if (m_rw)
    SDL_FreeRW(m_rw);
}
int Image::getOrientation(const char *data, size_t size)
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
Surface Image::resize(Surface surface, int outWidth, int outHeight, bool filter)
{
  if (!filter)
  {
    Surface newSurface = zoomSurface(surface, (float)outWidth / m_srcWidth, (float)outHeight / m_srcHeight, SMOOTHING_ON);
    return newSurface;
  }
  if (surface->format->format != SDL_PIXELFORMAT_RGBA32)
  {
    Surface convertedSurface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    if (convertedSurface == NULL)
    {
      return nullptr;
    }
    surface = convertedSurface;
  }
  size_t outStride = outWidth * 4;
  size_t outSize = outStride * outHeight;

  Surface dst(SDL_CreateRGBSurfaceWithFormat(0, outWidth, outHeight, 32, SDL_PIXELFORMAT_RGBA32));

  avir::CLancIR lancir;
  lancir.resizeImage(
      (uint8_t *)surface->pixels, surface->w, surface->h, 0,
      (uint8_t *)dst->pixels, outWidth, outHeight, 0,
      4);
  return dst;
}

Surface Image::getSurface(bool convert, bool filter)
{
  if (!m_rw)
  {
    return nullptr;
  }
  Surface surface;
  if (IMG_isSVG(m_rw))
  {
    auto document = Document::loadFromData(m_img->c_str(), m_img->length());
    if (!document.get())
      return nullptr;
    m_bitmap = document->renderToBitmap(m_width ? m_width : -1, m_height ? m_height : -1);
    m_bitmap.convertToRGBA();
    if (m_bitmap.isNull())
      return nullptr;
    surface = SDL_CreateRGBSurfaceWithFormatFrom(m_bitmap.data(), m_bitmap.width(), m_bitmap.height(), 32, m_bitmap.stride(), SDL_PIXELFORMAT_RGBA32);
  }
  else
  {
    surface = IMG_Load_RW(m_rw, 1);
  }
  m_srcWidth = surface->w;
  m_srcHeight = surface->h;
  if (!convert)
    return surface;

  if (m_srcWidth == 0 || m_srcHeight == 0)
  {
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

  Surface newSurface = resize(surface, outWidth, outHeight, filter);
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
    Surface rotatedSurface = rotozoomSurfaceXY(newSurface, angle, x, y, SMOOTHING_ON);
    newSurface = rotatedSurface;
  }
  return newSurface;
}
