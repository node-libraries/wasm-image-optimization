import WASM from "../esm/libImage.wasm?module";
import {
  _optimizeImage,
  _optimizeImageExt,
  type OptimizeParams,
} from "../lib/optimizeImage.js";
import LibImage from "../workers/libImage.js";

const libImage = LibImage({
  instantiateWasm: async (imports, receiver) => {
    receiver(await WebAssembly.instantiate(WASM, imports));
  },
});

export const optimizeImage = async (params: OptimizeParams) =>
  _optimizeImage({ ...params, libImage });

export const optimizeImageExt = async (params: OptimizeParams) =>
  _optimizeImageExt({ ...params, libImage });
