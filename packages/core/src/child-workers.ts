import { initWorker } from "worker-lib";
import { ImageConverter, OptimizeParams, OptimizeResult } from "./single.js";

let imageConverter: ImageConverter | undefined;

const getImageConverter = async () => {
  if (!imageConverter) {
    imageConverter = await ImageConverter.create();
  }
  return imageConverter;
};

/**
 * Worker-side implementation for Satoru.
 * Exposes Satoru methods via worker-lib.
 */
const actions = {
  async optimizeImage(options: OptimizeParams): Promise<OptimizeResult> {
    const s = await getImageConverter();
    return await s.optimizeImage(options);
  },
};

const map = initWorker(actions);
export type ImageConverterWorker = typeof map;
