# wasm-image-optimization

High-performance image conversion and resizing tool leveraging WebAssembly (Emscripten).
Built on the Skia rendering engine, it supports animations (GIF/WebP) and modern formats (AVIF/SVG/ThumbHash).

It provides a lightweight, dependency-free solution for generating high-quality visuals and documents across **Node.js**, **Cloudflare Workers**, **Deno**, and **Web Browsers**.

> [!TIP]
> If you need to generate image files from HTML/CSS, please use the [satoru-render](https://www.npmjs.com/package/satoru-render) library.


## Playground

https://node-libraries.github.io/wasm-image-optimization/

## Examples

https://github.com/SoraKumo001/wasm-image-optimization-samples

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

# Crop and resize
wasm-image-optimization input.png --crop 10,10,200,200 -w 100

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
  crop: { x: 0, y: 0, width: 400, height: 400 }, // specify crop region
  width: 800,
  height: 600,
  fit: 'contain', // 'contain' | 'cover' | 'fill' (default: 'contain')
  format: 'webp',  // 'none' | 'png' | 'webp' | 'jpeg' | 'avif' | 'raw' | 'thumbhash'
  quality: 85,
  speed: 6,       // 0-10 (encoding speed for AVIF, etc.)
  animation: true // maintain animation if possible
});
```

### Parameters (`OptimizeParams`)

| Parameter       | Type                                                      | Default      | Description                                                                                                                                                                      |
| :-------------- | :-------------------------------------------------------- | :----------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **`image`**     | `Uint8Array`<br>`ArrayBuffer`                             | **Required** | The raw binary data of the image to process.                                                                                                                                     |
| **`crop`**      | `{ x: number, y: number, width: number, height: number }` | -            | Rectangular region to crop before resizing. Values are in pixels relative to the original image.                                                                                 |
| **`width`**     | `number`                                                  | -            | Target width for resizing. If omitted, it's calculated from `height` to maintain aspect ratio.                                                                                   |
| **`height`**    | `number`                                                  | -            | Target height for resizing. If omitted, it's calculated from `width` to maintain aspect ratio.                                                                                   |
| **`fit`**       | `contain` <br> `cover` <br> `fill`                        | `'contain'`  | Resizing strategy. <br> - `contain`: Fits within bounds (aspect ratio preserved). <br> - `cover`: Fills bounds (may crop). <br> - `fill`: Stretches to match bounds exactly.     |
| **`format`**    | `string`                                                  | `'webp'`     | Output format: `'none'`, `'png'`, `'webp'`, `'jpeg'`, `'avif'`, `'raw'`, `'thumbhash'`. <br> _Note: If `animation: true` and the input is animated, this is forced to `'webp'`._ |
| **`quality`**   | `number`                                                  | `100`        | Compression quality (0-100). Applies to WebP, JPEG, and AVIF.                                                                                                                    |
| **`speed`**     | `number`                                                  | `6`          | Encoding speed (0-10). Primarily used for AVIF. Lower is slower but better compression.                                                                                          |
| **`animation`** | `boolean`                                                 | `false`      | If `true`, preserves animation frames (e.g., GIF/animated WebP source).                                                                                                          |

### Result (`OptimizeResult`)

| Property                | Type         | Description                                          |
| :---------------------- | :----------- | :--------------------------------------------------- |
| **`data`**              | `Uint8Array` | The processed image data.                            |
| **`originalWidth`**     | `number`     | Width of the input image.                            |
| **`originalHeight`**    | `number`     | Height of the input image.                           |
| **`originalAnimation`** | `boolean`    | `true` if the input image was animated.              |
| **`originalFormat`**    | `string`     | Format of the input image (e.g., `"jpeg"`, `"gif"`). |
| **`width`**             | `number`     | Width of the output image.                           |
| **`height`**            | `number`     | Height of the output image.                          |
| **`animation`**         | `boolean`    | `true` if the output image is animated.              |
| **`format`**            | `string`     | Format of the output image.                          |

### Logging & Debugging

You can monitor the internal processing and Skia logs.

```typescript
import { ImageConverter } from "wasm-image-optimization";

const converter = new ImageConverter();

// Set log level: 0 (None), 1 (Error), 2 (Warning), 3 (Info), 4 (Debug)
converter.logLevel = 3;

// Custom log handler
converter.onLog = (level, message) => {
  console.log(`[ImageConverter][${level}] ${message}`);
};
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
