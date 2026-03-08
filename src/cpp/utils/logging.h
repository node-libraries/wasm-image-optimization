#ifndef IMAGE_CONVERTER_UTILS_LOGGING_H
#define IMAGE_CONVERTER_UTILS_LOGGING_H

#include "../bridge/bridge_types.h"

// Define macros for easier logging.
// These call direct image_converter_log_printf call (printf-style).

#ifdef __cplusplus
extern "C" {
#endif
void image_converter_log_printf(LogLevel level, const char* format, ...);
#ifdef __cplusplus
}
#endif

#define IMAGE_CONVERTER_LOG_DEBUG(...) image_converter_log_printf(LogLevel::Debug, __VA_ARGS__)

#define IMAGE_CONVERTER_LOG_INFO(...) image_converter_log_printf(LogLevel::Info, __VA_ARGS__)

#define IMAGE_CONVERTER_LOG_WARN(...) image_converter_log_printf(LogLevel::Warning, __VA_ARGS__)

#define IMAGE_CONVERTER_LOG_ERROR(...) image_converter_log_printf(LogLevel::Error, __VA_ARGS__)

#endif  // IMAGE_CONVERTER_UTILS_LOGGING_H
