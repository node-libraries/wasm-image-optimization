import { createWorker } from "worker-lib";
import {
  _optimizeImage,
  _optimizeImageExt,
  type OptimizeParams,
} from "../lib/optimizeImage.js";
import type { WorkerType } from "./_web-worker.js";

const { execute, setLimit, close } = createWorker<WorkerType>(
  () => new Worker(new URL("../esm/next-web-worker.js", import.meta.url)),
  5
);

export const optimizeImage = async (params: OptimizeParams) =>
  execute("optimizeImage", params);

export const optimizeImageExt = async (params: OptimizeParams) =>
  execute("optimizeImageExt", params);

export { setLimit, close };
