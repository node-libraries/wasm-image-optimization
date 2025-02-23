import { initWorker } from "worker-lib/node";
import { optimizeImage, optimizeImageExt } from "./";

const map = initWorker({
  optimizeImage,
  optimizeImageExt,
});

export type WorkerType = typeof map;
