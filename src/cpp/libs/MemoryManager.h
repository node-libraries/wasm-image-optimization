#ifndef __MEMORYMANAGER_H__
#define __MEMORYMANAGER_H__

#include <emscripten/bind.h>

class MemoryManager
{
private:
  uint8_t *m_ptr;

public:
  MemoryManager();
  uint8_t *allocate(const uint8_t *data, size_t size);
  void release();
};
#endif
