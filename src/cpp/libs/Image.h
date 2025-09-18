#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <lunasvg.h>
#include "Surface.h"

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
  Image(std::string &img, float width, float height);
  ~Image();
  int getOrientation(const char *data, size_t size);
  Surface resize(Surface surface, int outWidth, int outHeight, bool filter);
  Surface getSurface(bool convert, bool filter);
  float getSrcWidth() const { return m_srcWidth; }
  float getSrcHeight() const { return m_srcHeight; }
};
#endif
