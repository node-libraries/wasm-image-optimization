import { createWorker } from "worker-lib";
import {
  _optimizeImage,
  _optimizeImageExt,
  type OptimizeParams,
} from "../lib/optimizeImage.js";
import type { WorkerType } from "../esm/_worker.js";

const execute = createWorker<WorkerType>(
  () => new Worker(new URL("../esm/worker.js", import.meta.url)),
  5
);

export const optimizeImage = async (params: OptimizeParams) =>
  execute("optimizeImage", params);

export const optimizeImageExt = async (params: OptimizeParams) =>
  execute("optimizeImageExt", params);
