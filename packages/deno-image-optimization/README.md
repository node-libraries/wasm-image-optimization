# deno-wasm-image-optimization

## usage

## Parameters

| Name | Type   | Description |
| ---- | ------ | ----------- |
| url  | string | Image URL   |
| q    | number | Quality     |
| w    | number | Width       |

https://xxxx.deno.dev/?url=https://xxx.png&q=80&w=200

- localhost
  http://localhost:8000/?url=https://raw.githubusercontent.com/SoraKumo001/cloudflare-ogp/refs/heads/master/sample/image.jpg&q=80

```ts
import { optimizeImage } from "npm:wasm-image-optimization";

const isValidUrl = (url: string) => {
  try {
    new URL(url);
    return true;
  } catch (_e) {
    return false;
  }
};

const isType = (accept: string | null, type: string) => {
  return (
    accept
      ?.split(",")
      .map((format) => format.trim())
      .some((format) => [`image/${type}`, "*/*", "image/*"].includes(format)) ??
    true
  );
};

Deno.serve(async (request) => {
  const url = new URL(request.url);
  const params = url.searchParams;
  const type = ["avif", "webp", "png", "jpeg"].find(
    (v) => v === params.get("type")
  ) as "avif" | "webp" | "png" | "jpeg" | undefined;
  const accept = request.headers.get("accept");
  const isAvif = isType(accept, "avif");
  const isWebp = isType(accept, "webp");

  const cache = await caches.open(
    `image-${isAvif ? "-avif" : ""}${isWebp ? "-webp" : ""}`
  );

  const cached = await cache.match(request);
  if (cached) {
    return cached;
  }

  const imageUrl = params.get("url");
  if (!imageUrl || !isValidUrl(imageUrl)) {
    return new Response("url is required", { status: 400 });
  }

  const cacheKey = new Request(url.toString());
  const cachedResponse = await cache.match(cacheKey);
  if (cachedResponse) {
    return cachedResponse;
  }

  const width = params.get("w");
  const quality = params.get("q");

  const [srcImage, contentType] = await fetch(imageUrl)
    .then(async (res) =>
      res.ok
        ? ([await res.arrayBuffer(), res.headers.get("content-type")] as const)
        : []
    )
    .catch(() => []);

  if (!srcImage) {
    return new Response("image not found", { status: 404 });
  }

  if (contentType && ["image/svg+xml", "image/gif"].includes(contentType)) {
    const response = new Response(srcImage, {
      headers: {
        "Content-Type": contentType,
        "Cache-Control": "public, max-age=31536000, immutable",
      },
    });
    cache.put(request, response.clone());
    return response;
  }

  const format =
    type ??
    (isAvif
      ? "avif"
      : isWebp
      ? "webp"
      : contentType === "image/jpeg"
      ? "jpeg"
      : "png");

  const image = await optimizeImage({
    image: srcImage,
    width: width ? Number(width) : undefined,
    quality: quality ? Number(quality) : undefined,
    format,
  });
  const response = new Response(image, {
    headers: {
      "Content-Type": `image/${format}`,
      "Cache-Control": "public, max-age=31536000, immutable",
      date: new Date().toUTCString(),
    },
  });
  cache.put(request, response.clone());
  return response;
});
```
