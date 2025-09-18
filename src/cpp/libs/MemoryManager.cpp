#include <emscripten/bind.h>
#include "MemoryManager.h"

MemoryManager::MemoryManager()
{
  m_ptr = nullptr;
}
uint8_t *MemoryManager::allocate(const uint8_t *data, size_t size)
{
  uint8_t *ptr = new uint8_t[size];
  memcpy(ptr, data, size);
  m_ptr = ptr;
  return ptr;
}
void MemoryManager::release()
{
  if (m_ptr)
  {
    delete[] m_ptr;
  }
}
