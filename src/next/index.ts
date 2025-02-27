import { _optimizeImage, _optimizeImageExt } from "../lib/optimizeImage.js";
import type { OptimizeParams, OptimizeResult } from "../esm/libImage.js";
export type { OptimizeParams, OptimizeResult };

const libImage = import("../esm/libImage.js").then((m) => m.default({}));

export const optimizeImage = async (params: OptimizeParams) =>
  _optimizeImage({ ...params, libImage });

export const optimizeImageExt = async (params: OptimizeParams) =>
  _optimizeImageExt({ ...params, libImage });
