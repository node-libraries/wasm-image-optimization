# wasm-image-optimization

- Cloudflare workers  
  `import { optimizeImage } from 'wasm-image-optimization';`
- Next.js(Webpack)  
  `import { optimizeImage } from 'wasm-image-optimization/next';`
- ESM(import base)  
  `import { optimizeImage } from 'wasm-image-optimization/esm';`
- CJS(file base)
  `import { optimizeImage } from 'wasm-image-optimization/node';`

- function

```ts
// quality: 1-100
optimizeImage({image: ArrayBuffer, width?: number, height?:number,quality?: number,format?: "png" | "jpeg" | "webp"}): Promise<ArrayBuffer>
```

- source format
  - jpeg(EXIF orientation is supported)
  - png
  - webp
  - svg
  - avif
- output format
  - jpeg
  - png
  - webp(default)

# usage

Next.js image optimization with Cloudflare

```ts
import { optimizeImage } from 'wasm-image-optimization';

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
  _env: {},
  ctx: ExecutionContext
): Promise<Response> => {
  const accept = request.headers.get('accept');
  const isWebp =
    accept
      ?.split(',')
      .map((format) => format.trim())
      .some((format) => ['image/webp', '*/*', 'image/*'].includes(format)) ?? true;

  const url = new URL(request.url);

  const params = url.searchParams;
  const imageUrl = params.get('url');
  if (!imageUrl || !isValidUrl(imageUrl)) {
    return new Response('url is required', { status: 400 });
  }

  const cache = caches.default;
  url.searchParams.append('webp', isWebp.toString());
  const cacheKey = new Request(url.toString());
  const cachedResponse = await cache.match(cacheKey);
  if (cachedResponse) {
    return cachedResponse;
  }

  const width = params.get('w');
  const quality = params.get('q');

  const [srcImage, contentType] = await fetch(imageUrl, { cf: { cacheKey: imageUrl } })
    .then(async (res) =>
      res.ok ? ([await res.arrayBuffer(), res.headers.get('content-type')] as const) : []
    )
    .catch(() => []);

  if (!srcImage) {
    return new Response('image not found', { status: 404 });
  }

  if (contentType && ['image/svg+xml', 'image/gif'].includes(contentType)) {
    const response = new Response(srcImage, {
      headers: {
        'Content-Type': contentType,
        'Cache-Control': 'public, max-age=31536000, immutable',
      },
    });
    ctx.waitUntil(cache.put(cacheKey, response.clone()));
    return response;
  }

  const format = isWebp ? 'webp' : contentType === 'image/jpeg' ? 'jpeg' : 'png';
  const image = await optimizeImage({
    image: srcImage,
    width: width ? parseInt(width) : undefined,
    quality: quality ? parseInt(quality) : undefined,
    format,
  });
  const response = new Response(image, {
    headers: {
      'Content-Type': `image/${format}`,
      'Cache-Control': 'public, max-age=31536000, immutable',
      date: new Date().toUTCString(),
    },
  });
  ctx.waitUntil(cache.put(cacheKey, response.clone()));
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
