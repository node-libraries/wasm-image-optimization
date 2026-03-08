#ifndef BRIDGE_TYPES_H
#define BRIDGE_TYPES_H

enum class LogLevel { None = 0, Error = 1, Warning = 2, Info = 3, Debug = 4 };
enum class RenderFormat { None = -1, SVG = 0, PNG = 1, WebP = 2, PDF = 3, JPEG = 4, AVIF = 5, RAW = 6, ThumbHash = 7 };
enum class FitMode { Contain = 0, Cover = 1, Fill = 2 };

struct RenderOptions {
    bool svgTextToPaths = true;
};

void image_converter_log(LogLevel level, const char *message);

#endif  // BRIDGE_TYPES_H
