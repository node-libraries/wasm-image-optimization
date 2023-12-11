# @node-libraries/wasm-webp-encoder

# Overview

Provides the ability to encode WebP using WebAssembly and WebWorker.  
Up to 4 parallel threads.

# types

```ts
encode(data: BufferSource, width: number, height: number, quality?: number): Promise<Uint8Array | null>
```

```ts
encode(data: ImageData, quality?: number): Promise<Uint8Array | null>
```

# usage

```ts
import { encode } from '@node-libraries/wasm-webp-encoder';

const encodedValue = await encode(ctx.getImageData(0, 0, img.width, img.height));
const encodedValue2 = await encode(arrayBuffer, width, height);
```

# Sample when used with Next.js

- https://github.com/SoraKumo001/next-webp
