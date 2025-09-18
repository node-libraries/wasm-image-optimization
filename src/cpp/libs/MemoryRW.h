#ifndef __MEMORYRW_H__
#define __MEMORYRW_H__

#include <emscripten/bind.h>
#include <SDL2/SDL_image.h>

class MemoryRW
{
public:
  MemoryRW();
  ~MemoryRW();
  operator SDL_RWops *() const;
  size_t size() const;
  const uint8_t *data() const;

protected:
  static size_t MemWrite(SDL_RWops *context, const void *ptr, size_t size, size_t num);
  static int MemClose(SDL_RWops *context);

private:
  SDL_RWops *m_rw;
  std::vector<uint8_t> m_buffer;
};
#endif
