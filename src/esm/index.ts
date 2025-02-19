import LibImage from "./libImage.js";
import {
  _optimizeImage,
  _optimizeImageExt,
  type OptimizeParams,
} from "../lib/optimizeImage.js";

const libImage = LibImage();

export const optimizeImage = async (params: OptimizeParams) =>
  _optimizeImage({ ...params, libImage });

export const optimizeImageExt = async (params: OptimizeParams) =>
  _optimizeImageExt({ ...params, libImage });
