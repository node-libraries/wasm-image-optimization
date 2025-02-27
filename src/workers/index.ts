import LibImage from "../cjs/libImage.js";
import WASM from "../esm/libImage.wasm";
import { _optimizeImage, _optimizeImageExt } from "../lib/optimizeImage.js";
import type { OptimizeParams, OptimizeResult } from "../types/index.js";
export type { OptimizeParams, OptimizeResult };

const libImage = LibImage({
  instantiateWasm: async (imports, receiver) => {
    receiver(await WebAssembly.instantiate(WASM, imports));
  },
});

export const optimizeImage = async (params: OptimizeParams) =>
  _optimizeImage({ ...params, libImage });

export const optimizeImageExt = async (params: OptimizeParams) =>
  _optimizeImageExt({ ...params, libImage });
