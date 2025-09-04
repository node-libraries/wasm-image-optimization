import fs from "node:fs";
import path from "node:path";
import type { Plugin, ResolvedConfig } from "vite";

export default (assetsPath?: string): Plugin[] => {
  let config: ResolvedConfig;
  return [
    {
      name: "wasm-image-optimization-serve",
      apply: "serve",
      config: () => {
        return {
          optimizeDeps: {
            exclude: ["wasm-image-optimization"],
          },
        };
      },
    },
    {
      name: "wasm-image-optimization-build",
      apply: "build",
      configResolved(_config) {
        config = _config;
      },
      writeBundle() {
        fs.copyFileSync(
          "node_modules/wasm-image-optimization/dist/esm/libImage.wasm",
          assetsPath
            ? path.resolve(assetsPath, "libImage.wasm")
            : path.resolve(config.build.outDir, "assets/libImage.wasm"),
        );
      },
    },
  ];
};
