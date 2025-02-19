import fs from "fs";
import path from "path";

// eslint-disable-next-line @typescript-eslint/ban-ts-comment
/* @ts-ignore */
import LibImage from "../../workers/libImage.js";
import {
  _optimizeImage,
  _optimizeImageExt,
  type OptimizeParams,
} from "../lib/optimizeImage.js";

const libImage = LibImage({
  wasmBinary: fs.readFileSync(
    path.resolve(__dirname, "../../esm/libImage.wasm")
  ) as never,
});

export const optimizeImage = async (params: OptimizeParams) =>
  _optimizeImage({ ...params, libImage });

export const optimizeImageExt = async (params: OptimizeParams) =>
  _optimizeImageExt({ ...params, libImage });
