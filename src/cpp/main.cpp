#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>

#include <string>
#include <vector>

#include "api/image_converter_api.h"

using namespace emscripten;

extern "C" {

EMSCRIPTEN_KEEPALIVE
static ImageConverterInstance* create_instance() { return api_create_instance(); }

EMSCRIPTEN_KEEPALIVE
static void destroy_instance(ImageConverterInstance* inst) { api_destroy_instance(inst); }
}

// Helper to convert JS Uint8Array or ArrayBuffer to std::vector<uint8_t>
static std::vector<uint8_t> val_to_vector(val data) {
    unsigned len = 0;
    if (data["length"].isUndefined()) {
        if (!data["byteLength"].isUndefined()) {
            len = data["byteLength"].as<unsigned>();
        }
    } else {
        len = data["length"].as<unsigned>();
    }

    std::vector<uint8_t> vec(len);
    if (len > 0) {
        val memoryView = val(typed_memory_view(len, vec.data()));
        if (data["length"].isUndefined()) {
            // It's an ArrayBuffer, so we need to create a view before calling set
            val uint8View = val::global("Uint8Array").new_(data);
            memoryView.call<void>("set", uint8View);
        } else {
            memoryView.call<void>("set", data);
        }
    }
    return vec;
}

static val load_image_val(ImageConverterInstance* inst, val data) {
    if (!inst) return val::null();
    auto vec = val_to_vector(data);
    if (api_load_image(inst, vec.data(), vec.size())) {
        return val(true);
    }
    return val(false);
}

static val encode_val(ImageConverterInstance* inst, int format, float quality, int speed, bool animation) {
    if (!inst) return val::null();
    int size = 0;
    const uint8_t* data = api_encode(inst, format, quality, speed, animation, size);
    if (!data || size == 0) return val::null();
    return val(typed_memory_view(size, data));
}

static bool crop_val(ImageConverterInstance* inst, int x, int y, int width, int height) {
    if (!inst) return false;
    return api_crop(inst, x, y, width, height);
}

static bool resize_val(ImageConverterInstance* inst, int width, int height, int fit) {
    if (!inst) return false;
    return api_resize(inst, width, height, fit);
}

EMSCRIPTEN_BINDINGS(image_converter) {
    class_<ImageConverterInstance>("ImageConverterInstance");

    function("create_instance", &create_instance, allow_raw_pointers());
    function("destroy_instance", &destroy_instance, allow_raw_pointers());
    function("load_image", &load_image_val, allow_raw_pointers());
    function("encode", &encode_val, allow_raw_pointers());
    function("crop", &crop_val, allow_raw_pointers());
    function("resize", &resize_val, allow_raw_pointers());
    function("get_original_width", &api_get_original_width, allow_raw_pointers());
    function("get_original_height", &api_get_original_height, allow_raw_pointers());
    function("is_original_animation", &api_is_original_animation, allow_raw_pointers());
    function("get_original_format", &api_get_original_format, allow_raw_pointers());
    function("get_width", &api_get_width, allow_raw_pointers());
    function("get_height", &api_get_height, allow_raw_pointers());
    function("is_animation", &api_is_animation, allow_raw_pointers());
    function("get_format", &api_get_format, allow_raw_pointers());
    function("set_log_level", &api_set_log_level);
}
