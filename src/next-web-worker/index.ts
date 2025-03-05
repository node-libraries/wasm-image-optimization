import { createWorker } from "worker-lib";
import type { WorkerType } from "./_web-worker.js";
import type { OptimizeParams, OptimizeResult } from "../types/index.js";
export type { OptimizeParams, OptimizeResult };

const { execute, setLimit, close, waitAll, waitReady, launchWorker } =
  createWorker<WorkerType>(
    () => new Worker(new URL("../esm/next-web-worker.js", import.meta.url)),
    5,
  );

export const optimizeImage = async (params: OptimizeParams) =>
  execute("optimizeImage", params);

export const optimizeImageExt = async (params: OptimizeParams) =>
  execute("optimizeImageExt", params);

export { setLimit, close, waitAll, waitReady, launchWorker };
