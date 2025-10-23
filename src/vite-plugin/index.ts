import type { Plugin, ResolvedConfig } from "vite";

export default (): Plugin[] => {
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
  ];
};
