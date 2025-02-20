import { createWorker } from "worker-lib";

import {
  _optimizeImage,
  _optimizeImageExt,
  type OptimizeParams,
} from "../lib/optimizeImage.js";

import type { WorkerType } from "./worker";

const execute = createWorker<WorkerType>(
  () => new Worker(new URL("./worker.js", import.meta.url)),
  5 // Maximum parallel number
);

export const optimizeImage = async (params: OptimizeParams) =>
  execute("optimizeImage", params);

export const optimizeImageExt = async (params: OptimizeParams) =>
  execute("optimizeImageExt", params);
