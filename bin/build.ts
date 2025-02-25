import { build } from "esbuild";

build({
  entryPoints: ["./dist/vite-web-worker/_web-worker.js"],
  outfile: "./dist/esm/vite-web-worker.js",
  target: "esnext",
  platform: "browser",
  external: ["./dist/esm/libImage.wasm"],
  bundle: true,
  format: "iife",
});

build({
  entryPoints: ["./dist/next-web-worker/_web-worker.js"],
  outfile: "./dist/esm/next-web-worker.js",
  target: "esnext",
  platform: "browser",
  external: ["./dist/esm/libImage.wasm"],
  bundle: true,
  format: "esm",
});

build({
  entryPoints: ["./dist/cjs/node/_node-worker.js"],
  outfile: "./dist/cjs/node/node-worker.js",
  target: "esnext",
  platform: "node",
  external: ["./dist/esm/libImage.wasm", "node:worker_threads"],
  bundle: true,
  format: "cjs",
});
