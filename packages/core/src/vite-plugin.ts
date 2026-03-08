import type { Plugin } from "vite";

export default (): Plugin[] => {
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
