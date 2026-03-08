# cloudflare-workers-image-optimization

Deploy the following code to Cloudflare Workers to receive requests for next/image in Next.js

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

const isType = (accept: string | null, type: string) => {
	return (
		accept
			?.split(',')
			.map((format) => format.trim())
			.some((format) => [`image/${type}`, '*/*', 'image/*'].includes(format)) ?? true
	);
};

const handleRequest = async (request: Request, _env: {}, ctx: ExecutionContext): Promise<Response> => {
	const url = new URL(request.url);
	const params = url.searchParams;
	const type = ['avif', 'webp', 'png', 'jpeg'].find((v) => v === params.get('type')) as 'avif' | 'webp' | 'png' | 'jpeg' | undefined;
	const accept = request.headers.get('accept');
	const isAvif = isType(accept, 'avif');
	const isWebp = isType(accept, 'webp');

	const cache = await caches.open(`img-${isAvif ? '-avif' : ''}${isWebp ? '-webp' : ''}`);

	const cached = await cache.match(request);
	if (cached) {
		return cached;
	}

	const imageUrl = params.get('url');
	if (!imageUrl || !isValidUrl(imageUrl)) {
		return new Response('url is required', { status: 400 });
	}

	const cacheKey = new Request(url.toString());
	const cachedResponse = await cache.match(cacheKey);
	if (cachedResponse) {
		return cachedResponse;
	}

	const width = params.get('w');
	const quality = params.get('q');

	const [srcImage, contentType] = await fetch(imageUrl, { cf: { cacheKey: imageUrl } })
		.then(async (res) => (res.ok ? ([await res.arrayBuffer(), res.headers.get('content-type')] as const) : []))
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

	const format = type ?? (isAvif ? 'avif' : isWebp ? 'webp' : contentType === 'image/jpeg' ? 'jpeg' : 'png');
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

## Deploy

```sh
wrangler deploy
```

## Setting of Next.js

To direct Next.js image optimization requests to Cloudflare Workers, set the following

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

## Local test

http://localhost:8787/?url=https://raw.githubusercontent.com/SoraKumo001/cloudflare-ogp/refs/heads/master/sample/image.jpg&q=80
