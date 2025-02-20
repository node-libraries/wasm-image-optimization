import { initWorker } from "worker-lib";
import { optimizeImage, optimizeImageExt } from "../vite/index.js";

// Initialization process to make it usable in Worker.
const map = initWorker({ optimizeImage, optimizeImageExt });
// Export only the type
export type WorkerType = typeof map;
