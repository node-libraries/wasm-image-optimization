#include "MemoryRW.h"

MemoryRW::MemoryRW()
{
  m_rw = SDL_AllocRW();
  m_rw->hidden.unknown.data1 = &m_buffer;
  m_rw->write = MemWrite;
  m_rw->close = MemClose;
}
MemoryRW::~MemoryRW()
{
  SDL_FreeRW(m_rw);
}
MemoryRW::operator SDL_RWops *() const { return m_rw; }
size_t MemoryRW::size() const { return m_buffer.size(); }
const uint8_t *MemoryRW::data() const { return m_buffer.data(); }

size_t MemoryRW::MemWrite(SDL_RWops *context, const void *ptr, size_t size, size_t num)
{
  std::vector<uint8_t> *buffer = (std::vector<uint8_t> *)context->hidden.unknown.data1;
  const uint8_t *bytes = (const uint8_t *)ptr;
  buffer->insert(buffer->end(), bytes, bytes + size * num);
  return num;
}
int MemoryRW::MemClose(SDL_RWops *context)
{
  return 0;
}
