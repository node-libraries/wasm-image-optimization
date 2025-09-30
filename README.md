# wasm-image-optimization

WebAssembly is used to provide image transformation functionality.

- Frontend

  - Next.js (Multithreading support)
  - React Router (Multithreading support)

- Backend

  - Cloudflare Workers
  - Deno Deploy
  - Node.js (Multithreading support)

- Convert
  - source format
    - svg
    - jpeg (EXIF orientation is supported)
    - png
    - webp
    - avif
  - output format
    - jpeg
    - png
    - webp
    - thumbhash
    - raw (Uncompressed (width*height*4))
    - avif (default)

## Example

https://next-image-convert.vercel.app/  
![](https://raw.githubusercontent.com/node-libraries/wasm-image-optimization/refs/heads/master/doc/image.webp)

## Functions

- Convert image

```ts
optimizeImage({
  image: BufferSource | Buffer | string; // The input image data
  width?: number; // The desired output width (optional)
  height?: number; // The desired output height (optional)
  quality?: number; // The desired output quality (0-100, optional)
  format?: "jpeg" | "png" | "webp" | "avif" | "thumbhash" | "raw" | "none"; // The desired output format (optional)
  speed?: number; // The desired speed (0-10, Slow-Fast, Optional)
  filter?: boolean; // resize filter (default true)
  ):
    Promise<
      ArrayBuffer>

optimizeImageExt({
  image: BufferSource | Buffer | string; // The input image data
  width?: number; // The desired output width (optional)
  height?: number; // The desired output height (optional)
  quality?: number; // The desired output quality (0-100, optional)
  format?: "jpeg" | "png" | "webp" | "avif" | "thumbhash" | "raw" | "none"; // The desired output format (optional)
  speed?: number; // The desired speed (0-10, Slow-Fast, Optional)
  filter?: boolean; // resize filter (default true)
  ):
    Promise<{
      data: Uint8Array,
      originalWidth: number,
      originalHeight: number,
      width: number,
      height: number}>

```

- Multi Threading

```ts
waitAll(): Promise<void>
waitReady(retryTime?: number) :Promise<void>
close(): void;
launchWorker(): Promise<void>
```

## WebWorker on Vite

To use Vite, the following settings are required

- vite.config.ts

```ts
import wasmImageOptimizationPlugin from "wasm-image-optimization/vite-plugin";

export default defineConfig(() => ({
  plugins: [
    wasmImageOptimizationPlugin(),
    //wasmImageOptimizationPlugin("build/client/assets") // optional: assetsPath
  ],
}));
```

## Supported Environments

- Cloudflare workers  
  `import { optimizeImage } from 'wasm-image-optimization';`
- Next.js (webpack)  
  `import { optimizeImage } from 'wasm-image-optimization/next';`
- ESM (import base) & Deno Deploy  
  `import { optimizeImage } from 'wasm-image-optimization';`
- Node.js  
  `import { optimizeImage } from 'wasm-image-optimization';`
- Node.js(Multi thread)  
  `import { optimizeImage } from 'wasm-image-optimization/node-worker';`
- Vite (Browser)  
  `import { optimizeImage } from 'wasm-image-optimization/vite';`
- Web Worker (Browser) Multi process  
  `import { optimizeImage } from 'wasm-image-optimization/web-worker';`

## Samples

https://github.com/SoraKumo001/wasm-image-optimization-samples
