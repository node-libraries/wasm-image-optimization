export type OptimizeResult = {
  data: Uint8Array;
  originalWidth: number;
  originalHeight: number;
  width: number;
  height: number;
};

export type OptimizeParams = {
  image: BufferSource | string; // The input image data
  width?: number; // The desired output width (optional)
  height?: number; // The desired output height (optional)
  quality?: number; // The desired output quality (0-100, optional)
  format?: "jpeg" | "png" | "webp" | "avif"; // The desired output format (optional)
  speed?: number; // The desired speed (0-10, Slow-Fast, Optional)
};

export declare type ModuleType = {
  optimize: (
    data: BufferSource | string,
    width: number,
    height: number,
    quality: number,
    format: "jpeg" | "png" | "webp" | "avif",
    speed: number
  ) => OptimizeResult | undefined;
  releaseResult: () => void;
};

declare const imageTools: (options?: {
  instantiateWasm?: (
    imports?: WebAssembly.Imports,
    receiver: (
      instance: WebAssembly.WebAssemblyInstantiatedSource
    ) => Promise<unknown>
  ) => void;
  locateFile?: (path: string, scriptDirectory: string) => string;
  wasmBinary?: ArrayBuffer;
}) => Promise<ModuleType>;
export default imageTools;
