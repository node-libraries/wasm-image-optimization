import path from "node:path";
import { Worker } from "node:worker_threads";
import { createWorker } from "worker-lib/node";
import {
  _optimizeImage,
  _optimizeImageExt,
  type OptimizeParams,
} from "../lib/optimizeImage.js";
import type { WorkerType } from "./_node-worker.js";

const { execute, waitAll, close } = createWorker<WorkerType>(() => {
  const url = path.resolve(__dirname, "./node-worker.js");
  return new Worker(url);
});

export const optimizeImage = async (params: OptimizeParams) =>
  execute("optimizeImage", params);

export const optimizeImageExt = async (params: OptimizeParams) =>
  execute("optimizeImageExt", params);

export { waitAll, close };
