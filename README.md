# wasm-image-optimization

High-performance image conversion and resizing tool leveraging WebAssembly (Emscripten).
Built on the Skia rendering engine, it supports animations (GIF/WebP) and modern formats (AVIF/SVG/ThumbHash).

## Playground

https://node-libraries.github.io/wasm-image-optimization/

## Features

- **Multi-format Support**: Supports major raster formats and SVG loading.
- **Animation Support**: Supports loading GIF and WebP animations. Animated WebP is recommended for output.
- **High-Quality Resizing**: Supports `fit` options (`contain`, `cover`, `fill`) with aspect ratio preservation.
- **Intelligent Animation Conversion**: Automatically switches the output format to `webp` if animations are enabled and the source is animated.
- **Fast Execution**: High-speed image processing using C++20 and SIMD (`msimd128`).
- **TypeScript Ready**: Provides type-safe wrappers for easy integration.
- **Multi-threaded**: Supports parallel processing using Web Workers / Worker Threads.
- **Multi-environment**: Works seamlessly across Node.js, Cloudflare Workers, Deno, and Browsers.
- **Lightweight**: Optimized binary size (approx. 7.4MB for the single-file bundle) by removing unnecessary dependencies like FreeType.

## Supported Formats

| Format        | Input (Decoding) | Output (Encoding) | Notes                                       |
| :------------ | :--------------: | :---------------: | :------------------------------------------ |
| **PNG**       |        ✅        |        ✅         |                                             |
| **JPEG**      |        ✅        |        ✅         |                                             |
| **WebP**      |        ✅        |        ✅         | Supports animations                         |
| **AVIF**      |        ✅        |        ✅         | Powered by AOM encoder / dav1d decoder      |
| **ThumbHash** |        ✅        |        ✅         | Returns a compact image placeholder hash    |
| **RAW**       |        ✅        |        ✅         | Returns uncompressed 32-bit RGBA pixel data |
| **GIF**       |        ✅        |         -         | Input only (supports animations)            |
| **SVG**       |        ✅        |         -         | High-fidelity rendering via SkSVGDOM        |
| **BMP**       |        ✅        |         -         |                                             |
| **None**      |        ✅        |        ✅         | Returns original data with metadata         |

## Installation

```bash
pnpm add wasm-image-optimization
```

## CLI Usage

After installation, the `wasm-image-optimization` command will be available.

```bash
# Basic conversion (default is WebP)
wasm-image-optimization input.png

# Specify format and quality
wasm-image-optimization input.jpg -f avif -q 80

# Convert maintaining animation
wasm-image-optimization input.gif -f webp -a

# Specify encoding speed (e.g., for AVIF)
wasm-image-optimization input.png -f avif -s 0

# Resize and save with a different name
wasm-image-optimization input.png -w 800 -h 600 -o resized.webp
```

## Library Usage (TypeScript)

### Basic Usage

```typescript
import { optimizeImage } from 'wasm-image-optimization';


const inputBuffer = ...; // Uint8Array or ArrayBuffer

const result = await optimizeImage({
  image: inputBuffer,
  width: 800,
  height: 600,
  fit: 'contain', // 'contain' | 'cover' | 'fill' (default: 'contain')
  format: 'webp',  // 'none' | 'png' | 'webp' | 'jpeg' | 'avif' | 'raw' | 'thumbhash'
  quality: 85,
  speed: 6,       // 0-10 (encoding speed for AVIF, etc.)
  animation: true // maintain animation if possible
});

// Result structure
// {
//   data: Uint8Array,           // processed image data
//   originalWidth: number,      // source width
//   originalHeight: number,     // source height
//   originalAnimation: boolean, // true if source was animated
//   originalFormat: string,     // source format (e.g., "jpeg")
//   width: number,              // output width
//   height: number,             // output height
//   animation: boolean,         // true if output is animated
//   format: string              // output format
// }
```

### Multi-threading (Workers)

Use Workers for high-throughput processing without blocking the main thread.

```typescript
import { optimizeImage } from "wasm-image-optimization/workers";

const result = await optimizeImage({
  image: inputBuffer,
  width: 800,
  format: "png",
});
```

### Cloudflare Workers

Use the specialized `workerd` entry point for edge runtimes.

```typescript
import { optimizeImage } from "wasm-image-optimization/workerd";

const result = await optimizeImage({
  image: inputBuffer,
  width: 800,
  format: "avif",
});
```

## Build

### Prerequisites

- **Emscripten (emsdk)**: Ensure the latest version is installed and environment variables are set.
- **vcpkg**: Used for managing C++ dependencies.

### Steps

```bash
# Install dependencies
pnpm install

# Configure CMake (using Ninja)
pnpm wasm:configure

# Build Wasm
pnpm wasm:build
```

## Tech Stack

- **Core**: Skia, libavif (AOM / dav1d), Wuffs, expat
- **Runtime**: WebAssembly (Emscripten)
- **Language**: C++20, TypeScript

## License

MIT
