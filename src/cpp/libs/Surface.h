#ifndef __SURFACE_H__
#define __SURFACE_H__

#include <emscripten/bind.h>
#include <SDL2/SDL_image.h>

class Surface
{
public:
  Surface()
  {
  }
  Surface(SDL_Surface *surface)
  {
    m_surface = std::shared_ptr<SDL_Surface>(surface, SDL_FreeSurface);
  }
  void release()
  {
    m_surface = std::shared_ptr<SDL_Surface>(nullptr);
  }
  SDL_Surface *operator->() const
  {
    return m_surface.get();
  }
  operator SDL_Surface *() const { return m_surface.get(); }
  explicit operator bool() const { return m_surface != nullptr; }

private:
  std::shared_ptr<SDL_Surface> m_surface;
};
#endif
