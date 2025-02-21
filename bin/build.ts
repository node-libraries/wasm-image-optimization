import { build } from "esbuild";

build({
  entryPoints: ["./dist/esm/_worker.js"],
  outfile: "./dist/esm/worker.js",
  target: "esnext",
  platform: "browser",
  external: ["./dist/esm/libImage.wasm"],
  bundle: true,
  format: "iife",
});
