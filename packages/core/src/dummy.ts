import type { OptimizeParams, OptimizeResult } from "./core";
export type { OptimizeParams, OptimizeResult };

export const optimizeImage = (_params: OptimizeParams): OptimizeResult => {
  return undefined as never;
};

export const setLimit = (_limit: number): void => {};
export const close = () => {};
export const waitAll = () => Promise.resolve();
export const waitReady = (_retryTime?: number) => Promise.resolve();
export const launchWorker = () => Promise.resolve();
