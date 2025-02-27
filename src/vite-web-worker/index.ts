import { createWorker } from "worker-lib";
import { _optimizeImage, _optimizeImageExt } from "../lib/optimizeImage.js";
import type { WorkerType } from "./_web-worker.js";
import type { OptimizeParams, OptimizeResult } from "../esm/libImage.js";
export type { OptimizeParams, OptimizeResult };

const { execute, setLimit, close } = createWorker<WorkerType>(
  () => new Worker(new URL("../esm/vite-web-worker.js", import.meta.url)),
  5
);

export const optimizeImage = async (params: OptimizeParams) =>
  execute("optimizeImage", params);

export const optimizeImageExt = async (params: OptimizeParams) =>
  execute("optimizeImageExt", params);

export { setLimit, close };
