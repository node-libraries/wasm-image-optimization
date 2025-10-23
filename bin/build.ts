import { build } from "esbuild";

build({
  entryPoints: ["./dist/web-worker/_web-worker.js"],
  outfile: "./dist/esm/web-worker.js",
  target: "esnext",
  platform: "browser",
  bundle: true,
  format: "esm",
  define: { "import.meta.url": '""' },
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
