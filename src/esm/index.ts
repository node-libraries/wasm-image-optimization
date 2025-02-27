import LibImage, {
  type OptimizeParams,
  type OptimizeResult,
} from "./libImage.js";
import { _optimizeImage, _optimizeImageExt } from "../lib/optimizeImage.js";

export type { OptimizeParams, OptimizeResult };

const libImage = LibImage();

export const optimizeImage = async (params: OptimizeParams) =>
  _optimizeImage({ ...params, libImage });

export const optimizeImageExt = async (params: OptimizeParams) =>
  _optimizeImageExt({ ...params, libImage });
