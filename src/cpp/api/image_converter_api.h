#ifndef IMAGE_CONVERTER_API_H
#define IMAGE_CONVERTER_API_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "../bridge/bridge_types.h"
#include "core/image_converter_context.h"
#include "include/core/SkBitmap.h"

struct ImageFrame {
    SkBitmap bitmap;
    int duration;
};

class ImageConverterInstance {
   public:
    ImageConverterContext context;
    std::vector<ImageFrame> frames;
    sk_sp<SkData> original_data;
    int original_width = 0;
    int original_height = 0;
    bool original_animation = false;
    std::string original_format;
    bool output_animation = false;
    std::string output_format;

    ImageConverterInstance();
    ~ImageConverterInstance();

    bool load_image(const uint8_t* data, size_t size);
    const uint8_t* encode(RenderFormat format, float quality, int speed, bool animation, int& out_size);
    bool resize(int width, int height, FitMode fit = FitMode::Contain);

    int get_original_width() const;
    int get_original_height() const;
    bool is_original_animation() const;
    std::string get_original_format() const;
    int get_width() const;
    int get_height() const;
    bool is_animation() const;
    std::string get_format() const;
};

ImageConverterInstance* api_create_instance();
void api_destroy_instance(ImageConverterInstance* inst);

const uint8_t* api_load_image(ImageConverterInstance* inst, const uint8_t* data, size_t size);
const uint8_t* api_encode(ImageConverterInstance* inst, int format, float quality, int speed, bool animation, int& out_size);
bool api_resize(ImageConverterInstance* inst, int width, int height, int fit = 0);

int api_get_original_width(ImageConverterInstance* inst);
int api_get_original_height(ImageConverterInstance* inst);
bool api_is_original_animation(ImageConverterInstance* inst);
std::string api_get_original_format(ImageConverterInstance* inst);
int api_get_width(ImageConverterInstance* inst);
int api_get_height(ImageConverterInstance* inst);
bool api_is_animation(ImageConverterInstance* inst);
std::string api_get_format(ImageConverterInstance* inst);

void api_set_log_level(int level);

#endif  // IMAGE_CONVERTER_API_H
