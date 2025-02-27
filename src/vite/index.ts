import LibImage, { type ModuleType } from "../cjs/libImage.js";
import WASM from "../esm/libImage.wasm?url";
import { _optimizeImage, _optimizeImageExt } from "../lib/optimizeImage.js";
import type { OptimizeParams, OptimizeResult } from "../types/index.js";
export type { OptimizeParams, OptimizeResult };

let libImage: Promise<ModuleType>;
const getLibImage = async () => {
  if (!libImage) {
    libImage = LibImage({
      wasmBinary: await fetch(WASM).then((v) => v.arrayBuffer()),
    });
  }
  return libImage;
};

export const optimizeImage = (params: OptimizeParams) =>
  _optimizeImage({ ...params, libImage: getLibImage() });

export const optimizeImageExt = (params: OptimizeParams) =>
  _optimizeImageExt({ ...params, libImage: getLibImage() });
