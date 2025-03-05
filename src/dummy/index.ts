import { _optimizeImage, _optimizeImageExt } from "../lib/optimizeImage.js";
import type { OptimizeParams, OptimizeResult } from "../types/index.js";
export type { OptimizeParams, OptimizeResult };

export const optimizeImage = (_params: OptimizeParams): OptimizeResult => {
  return undefined as never;
};
export const optimizeImageExt = (_params: OptimizeParams): OptimizeResult => {
  return undefined as never;
};
export const setLimit = (_limit: number): void => {};
export const close = () => {};
export const waitAll = () => Promise.resolve();
export const waitReady = (_retryTime?: number) => Promise.resolve();
export const launchWorker = () => Promise.resolve();
