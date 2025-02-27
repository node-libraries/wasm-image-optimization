import { initWorker } from "worker-lib";
import { _optimizeImage, _optimizeImageExt } from "../lib/optimizeImage.js";
import type { OptimizeParams } from "../types/index.js";

const libImage = import("../cjs/libImage.js").then((m) => m.default({}));

const map = initWorker({
  optimizeImage: async (params: OptimizeParams) =>
    _optimizeImage({ ...params, libImage }),
  optimizeImageExt: async (params: OptimizeParams) =>
    _optimizeImageExt({ ...params, libImage }),
});

export type WorkerType = typeof map;
