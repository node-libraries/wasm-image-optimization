import { initWorker } from "worker-lib";
import {
  _optimizeImage,
  _optimizeImageExt,
  type OptimizeParams,
} from "../lib/optimizeImage.js";

const libImage = import("../workers/libImage.js").then((m) => m.default({}));

const map = initWorker({
  optimizeImage: async (params: OptimizeParams) =>
    _optimizeImage({ ...params, libImage }),
  optimizeImageExt: async (params: OptimizeParams) =>
    _optimizeImageExt({ ...params, libImage }),
});

export type WorkerType = typeof map;
