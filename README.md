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
    - avif (default)

## Example

https://next-image-convert.vercel.app/  
![](https://raw.githubusercontent.com/node-libraries/wasm-image-optimization/refs/heads/master/doc/image.avif)

## Functions

- Convert image

```ts
optimizeImage({
  image: ArrayBuffer,
  width?: number,
  height?:number,
  quality?: number, // quality: 1-100
  format?: "png" | "jpeg" | "avif" | "webp" | "none"}
  speed?: number // avif-speed: 0-10 (Slow-Fast, default: 6)
  ):
    Promise<
      ArrayBuffer>

optimizeImageExt({
  image: ArrayBuffer,
  width?: number,
  height?:number,
  quality?: number,
  format?: "png" | "jpeg" | "avif" | "webp" | "none"}
  speed?: number // avif-speed: 0-10 (Slow-Fast, default: 6)
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
close():void;
```

## WebWorker on Vite

To use Vite, the following settings are required

- vite.config.ts

```ts
import wasmImageOptimizationPlugin from "wasm-image-optimization/vite-plugin";

export default defineConfig(() => ({
  plugins: [wasmImageOptimizationPlugin()],
}));
```

## Supported Environments

- Cloudflare workers  
  `import { optimizeImage } from 'wasm-image-optimization';`
- Next.js (Webpack)  
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
