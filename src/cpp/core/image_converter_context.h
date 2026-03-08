#ifndef IMAGE_CONVERTER_CONTEXT_H
#define IMAGE_CONVERTER_CONTEXT_H

#include <memory>
#include <vector>

#include "include/core/SkBitmap.h"
#include "include/core/SkData.h"

class ImageConverterContext {
    sk_sp<SkData> m_lastOutput;

public:
    ImageConverterContext() {}

    void init();

    void set_last_output(sk_sp<SkData> data) { m_lastOutput = std::move(data); }
    const sk_sp<SkData>& get_last_output() const { return m_lastOutput; }
    size_t get_last_output_size() const { return m_lastOutput ? m_lastOutput->size() : 0; }
};

#endif // IMAGE_CONVERTER_CONTEXT_H
