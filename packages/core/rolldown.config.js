import { defineConfig } from "rolldown";

export default defineConfig({
  input: "src/child-workers.ts",
  output: {
    file: "dist/web-workers.js",
    format: "esm",
    codeSplitting: false,
  },
  resolve: {
    extensionAlias: {
      ".js": [".ts", ".js"],
    },
  },
  external: [/node:.*/, "worker_threads"],
});
