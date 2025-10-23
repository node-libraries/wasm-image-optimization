import fs from "node:fs";
import path from "node:path";
// eslint-disable-next-line @typescript-eslint/ban-ts-comment
/* @ts-ignore */
import LibImage from "../../cjs/libImage.js";
import { _optimizeImage, _optimizeImageExt } from "../lib/optimizeImage.js";
import type { OptimizeParams, OptimizeResult } from "../types/index.js";
export type { OptimizeParams, OptimizeResult };

const libImage = LibImage({
  wasmBinary: fs.readFileSync(
    path.resolve(__dirname, "../../cjs/libImage.wasm")
  ) as never,
});

export const optimizeImage = async (params: OptimizeParams) =>
  _optimizeImage({ ...params, libImage });

export const optimizeImageExt = async (params: OptimizeParams) =>
  _optimizeImageExt({ ...params, libImage });
