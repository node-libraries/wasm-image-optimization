import { Worker } from "node:worker_threads";
import { createWorker } from "worker-lib/node";
import {
  _optimizeImage,
  _optimizeImageExt,
  type OptimizeParams,
} from "../lib/optimizeImage.js";
import type { WorkerType } from "./_web-worker.js";

const { execute, waitAll, close } = createWorker<WorkerType>(() => {
  return new Worker(new URL("./web-worker.js", import.meta.url));
});

export const optimizeImage = async (params: OptimizeParams) =>
  execute("optimizeImage", params);

export const optimizeImageExt = async (params: OptimizeParams) =>
  execute("optimizeImageExt", params);

export { waitAll, close };
