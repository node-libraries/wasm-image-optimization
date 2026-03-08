# wasm-image-optimization: High-Performance Image Optimization

[![Playground](https://img.shields.io/badge/Demo-Playground-blueviolet)](https://sorakumo001.github.io/image_converter/)
[![npm license](https://img.shields.io/npm/l/wasm-image-optimization.svg)](https://www.npmjs.com/package/wasm-image-optimization)
[![npm version](https://img.shields.io/npm/v/wasm-image-optimization.svg)](https://www.npmjs.com/package/wasm-image-optimization)
[![npm download](https://img.shields.io/npm/dw/wasm-image-optimization.svg)](https://www.npmjs.com/package/wasm-image-optimization)
[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/SoraKumo001/image_converter)

**Image Converter** is a high-fidelity image-to-image conversion engine built with WebAssembly. It provides a lightweight, dependency-free solution for generating high-quality visuals across **Node.js**, **Cloudflare Workers**, **Deno**, and **Web Browsers**.

By leveraging the **Skia** graphics engine, Image Converter performs all drawing operations entirely within WASM.

![Sample Output](./document/sample01.webp)

## Example

- Cloudflare Workers  
  https://github.com/SoraKumo001/image_converter-cloudflare-ogp
- Deno Deploy  
  https://github.com/SoraKumo001/image_converter-deno-ogp-image
- Next.js(Vercel)  
  https://github.com/SoraKumo001/next-image_converter

---

## 📋 Supported CSS Properties

<details>
<summary>Click to expand supported properties list</summary>

### Box Model, Layout & Logical Properties

- `display` (block, inline, flex, grid, list-item, table, etc.)
- `position` (static, relative, absolute, fixed)
- `float`, `clear`, `visibility`, `z-index`, `overflow`, `box-sizing`, `aspect-ratio`
- `width`, `height`, `min-width`, `min-height`, `max-width`, `max-height`
- `margin`, `padding`, `border` (Width, Style, Color)
- **Logical Properties**: `inline-size`, `block-size`, `margin-inline`, `margin-block`, `padding-inline`, `padding-block`, `border-inline`, `border-block` (Start/End)

### Typography & Text

- `color`, `font-family`, `font-size`, `font-weight`, `font-style`, `line-height`
- `text-align`, `vertical-align`, `text-decoration` (Underline, Overline, Line-through, Wavy)
- `text-transform`, `text-indent`, `text-overflow` (Ellipsis), `white-space`, `line-clamp`
- `text-shadow`, `direction`, `writing-mode` (horizontal-tb, vertical-rl, vertical-lr)

### Backgrounds, Borders & Shadows

- `background` (Color, Image, Position, Size, Repeat, Clip, Origin)
- `border-radius`, `box-shadow` (Outer & Inset)
- `border-image` (Source, Slice, Width, Outset, Repeat)

### Flexbox & Grid

- `display: flex`, `flex-direction`, `flex-wrap`, `justify-content`, `align-items`, `align-content`, `align-self`, `flex-grow`, `flex-shrink`, `flex-basis`, `gap`, `order`
- `display: grid`, `grid-template-columns`, `grid-template-rows`, `grid-column`, `grid-row`, `gap`

### Effects, Shapes & Functions

- `clip-path` (circle, ellipse, inset, polygon, path)
- `filter`, `backdrop-filter`, `opacity`
- **Gradients**: `linear-gradient`, `radial-gradient`, `conic-gradient`
- **Modern Functions**: `calc()`, `min()`, `max()`, `clamp()`, `oklch()`, `oklab()`, `color-mix()`, `light-dark()`, `env()`, `var()`
- **Container Queries**: `@container`, `container-type`, `container-name`
- **Masking**: `mask`, `-webkit-mask`
- `content`, `appearance`

</details>

## 📦 Installation

```bash
npm install wasm-image-optimization
```

---

## 🚀 Quick Start

### Basic Usage (TypeScript)

The `render` function is the primary entry point. It handles WASM instantiation, resource resolution, and conversion in a single call.

```typescript
import { optimizeImage } from "wasm-image-optimization";

const html = `
  <div style="padding: 40px; background: #f8f9fa; border-radius: 12px; border: 2px solid #dee2e6;">
    <h1 style="color: #007bff; font-family: sans-serif;">Hello image_converter!</h1>
    <p style="color: #495057;">This document was rendered entirely in WebAssembly.</p>
  </div>
`;

const png = await optimizeImage({
  image: buffer,
  width: 600,
  format: "png",
});
```

---

## 🛠�E�EAdvanced Usage

### 1. Dynamic Resource Resolution

image_converter can automatically fetch missing fonts, images, or external CSS via a `resolveResource` callback.

```typescript
const pdf = await render({
  value: html,
  width: 800,
  format: "pdf",
  baseUrl: "https://example.com/assets/",
  resolveResource: async (resource, defaultResolver) => {
    // Custom intercept logic
    if (resource.url.startsWith("my-app://")) {
      return myAssetBuffer;
    }
    // Fallback to default fetch/filesystem resolver
    return defaultResolver(resource);
  },
});
```

### 2. Multi-page PDF Generation

Generate complex documents by passing an array of HTML strings. Each element in the array becomes a new page.

```typescript
const pdf = await render({
  value: ["<h1>Page One</h1>", "<h1>Page Two</h1>", "<h1>Page Three</h1>"],
  width: 595, // A4 width in points
  format: "pdf",
});
```

### 3. Edge/Cloudflare Workers

Use the specialized `workerd` export for serverless environments.

```typescript
import { optimizeImage } from "wasm-image-optimization";

export default {
  async fetch(request) {
    const png = await optimizeImage({
      image: buffer,
      width: 800,
      format: "png",
    });

    return new Response(png, { headers: { "Content-Type": "image/png" } });
  },
};
```

### 4. Multi-threaded Rendering (Worker Proxy)

Distribute rendering tasks across multiple background workers for high-throughput applications.

```typescript
import { createImageConverterWorker } from "wasm-image-optimization/workers";

const image_converter = createImageConverterWorker({ maxParallel: 4 });

const png = await image_converter.optimizeImage({
  image: buffer,
  width: 800,
  format: "png",
});
```

### 5. preact + tailwind

- install

```bash
pnpm add wasm-image-optimization
```

- code

```tsx
/** @jsx h */
import { h, toHtml } from "wasm-image-optimization/preact";
import { createCSS } from "wasm-image-optimization/tailwind";
import { optimizeImage } from "wasm-image-optimization";

// 1. Define your layout with Tailwind classes
const html = toHtml(
  <div className="w-[1200px] h-[630px] flex items-center justify-center bg-slate-900">
    <h1 className="text-6xl text-white font-bold">Hello World</h1>
  </div>,
);

// 2. Generate CSS from the HTML
const css = await createCSS(html);

// 3. Render to PNG
const png = await render({
  value: html,
  css,
  width: 1200,
  height: 630,
  format: "png",
});
```

- tsconfig.json

```json
{
  "compilerOptions": {
    "jsx": "preserve"
  }
}
```

---

## 💻 CLI Tool

Convert files or URLs directly from your terminal.

```bash
# Local file to PNG
npx wasm-image-optimization input.png -o output.png

# WebP conversion with quality
npx wasm-image-optimization input.png -f webp -q 80
```

---

## 📖 API Reference

### Render Options

| Option            | Type                                | Description                                             |
| :---------------- | :---------------------------------- | :------------------------------------------------------ |
| `value`           | `string \| string[]`                | HTML string or array of strings (for multi-page PDF).   |
| `url`             | `string`                            | URL to fetch HTML from.                                 |
| `width`           | `number`                            | **Required.** Output width in pixels.                   |
| `height`          | `number`                            | Output height. Default: `0` (auto-calculate).           |
| `format`          | `"svg" \| "png" \| "webp" \| "pdf"` | Output format. Default: `"svg"`.                        |
| `resolveResource` | `ResourceResolver`                  | Async callback to fetch assets (fonts, images, CSS).    |
| `fonts`           | `Object[]`                          | Pre-load fonts: `[{ name, data: Uint8Array }]`.         |
| `css`             | `string`                            | Extra CSS to inject into the document.                  |
| `baseUrl`         | `string`                            | Base URL for relative path resolution.                  |
| `logLevel`        | `LogLevel`                          | Verbosity: `None`, `Error`, `Warning`, `Info`, `Debug`. |

---

## 📜 License

This project is licensed under the **MIT License**.
