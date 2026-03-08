#ifndef THUMBHASH_H
#define THUMBHASH_H

#include <vector>
#include <cstdint>

std::vector<uint8_t> rgbaToThumbHash(int w, int h, const uint8_t *rgba);

#endif // THUMBHASH_H
