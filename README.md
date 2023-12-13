# wasm-image-optimization

- Cloudflare workers  
  `import { optimizeImage } from 'wasm-image-optimization';`
- Edge runtime  
  `import { optimizeImage } from 'wasm-image-optimization/edge';`
- Node runtime  
  `import { optimizeImage } from 'wasm-image-optimization/node';`

- function

```ts
// quality: 1-100
optimizeImage({image: ArrayBuffer, width?: number, height?:number,quality?: number}): Promise<ArrayBuffer>
```

- source format
  - jpeg
  - png
  - webp
  - svg
- output format
  - webp

# usage

Next.js image optimization with Cloudflare

```ts
import { optimizeImage } from 'wasm-image-optimization';
export interface Env {}

const isValidUrl = (url: string) => {
  try {
    new URL(url);
    return true;
  } catch (err) {
    return false;
  }
};

const handleRequest = async (
  request: Request,
  _env: Env,
  ctx: ExecutionContext
): Promise<Response> => {
  const url = new URL(request.url);
  const params = url.searchParams;
  const imageUrl = params.get('url');
  if (!imageUrl || !isValidUrl(imageUrl)) {
    return new Response('url is required', { status: 400 });
  }
  const cache = caches.default;
  const cachedResponse = await cache.match(new Request(url.toString(), request));
  if (cachedResponse) {
    return cachedResponse;
  }

  const width = params.get('w');
  const quality = params.get('q');

  const srcImage = await fetch(imageUrl, { cf: { cacheKey: imageUrl } })
    .then((res) => (res.ok ? res.arrayBuffer() : null))
    .catch((e) => null);

  if (!srcImage) {
    return new Response('image not found', { status: 404 });
  }
  const image = await optimizeImage({
    image: srcImage,
    width: width ? parseInt(width) : undefined,
    quality: quality ? parseInt(quality) : undefined,
  });
  const response = new Response(image, {
    headers: {
      'Content-Type': 'image/webp',
      'Cache-Control': 'public, max-age=31536000, immutable',
    },
  });
  ctx.waitUntil(cache.put(request, response.clone()));
  return response;
};

export default {
  fetch: handleRequest,
};
```

- next.config.js

```js
/**
 * @type { import("next").NextConfig}
 */
const config = {
  images: {
    path: 'https://xxx.yyy.workers.dev/',
  },
};
export default config;
```
