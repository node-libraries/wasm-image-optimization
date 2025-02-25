# wasm-image-optimization

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

### cloudflare-ogp

Sample for generating OGP images on Cloudflare Workers.

### deno-ogp

Sample for generating OGP images on Deno.

### cloudflare-image-optimization

Sample for image optimization on Cloudflare Workers.

### deno-image-optimization

Sample for image optimization on Deno.

### node-image-convert

Sample for converting image formats on Node.js.  
Single-threaded and multi-threaded operation can be selected.

## WebWorker on Vite

- vite.config.ts

```ts
import wasmImageOptimizationPlugin from "wasm-image-optimization/vite-plugin";

export default defineConfig(() => ({
  plugins: [wasmImageOptimizationPlugin()],
}));
```

## Next.js on Backend API

```ts
import { optimizeImage } from "wasm-image-optimization";
…
export const runtime = "edge";
```

## function

```ts

optimizeImage({
  image: ArrayBuffer,
  width?: number,
  height?:number,
  quality?: number, // quality: 1-100
  format?: "png" | "jpeg" | "avif" | "webp"})
    :Promise<
      ArrayBuffer>

optimizeImageExt({
  image: ArrayBuffer,
  width?: number,
  height?:number,
  quality?: number,
  format?: "png" | "jpeg" | "avif" | "webp"})
    :Promise<{
      data: Uint8Array;
      originalWidth: number;
      originalHeight: number;
      width: number;
      height: number;}>

```

- source format
  - svg
  - jpeg(EXIF orientation is supported)
  - png
  - webp
  - avif
- output format
  - jpeg
  - png
  - webp
  - avif(default)
