import LibImage from "../cjs/libImage.js";
import { _optimizeImage, _optimizeImageExt } from "../lib/optimizeImage.js";
import type { OptimizeParams, OptimizeResult } from "../types/index.js";
export type { OptimizeParams, OptimizeResult };

const getLibImage = async () =>
  LibImage({
    wasmBinary: await fetch(
      "https://cdn.jsdelivr.net/npm/wasm-image-optimization@1.2.18/dist/esm/libImage.wasm",
    ).then((v) => v.arrayBuffer()),
  });

export const optimizeImage = async (params: OptimizeParams) =>
  _optimizeImage({ ...params, libImage: getLibImage() });

export const optimizeImageExt = async (params: OptimizeParams) =>
  _optimizeImageExt({ ...params, libImage: getLibImage() });

export const setLimit = (_limit: number): void => {};
export const close = () => {};
export const waitAll = () => Promise.resolve();
export const waitReady = (_retryTime?: number) => Promise.resolve();
