export declare type ModuleType = {
  optimize: (
    data: BufferSource | string,
    width: number,
    height: number,
    quality: number,
    format: "jpeg" | "png" | "webp" | "avif" | "none",
    speed: number,
  ) => OptimizeResult | undefined;
  releaseResult: () => void;
};

declare const imageTools: (options?: {
  instantiateWasm?: (
    imports?: WebAssembly.Imports,
    receiver: (
      instance: WebAssembly.WebAssemblyInstantiatedSource,
    ) => Promise<unknown>,
  ) => void;
  locateFile?: (path: string, scriptDirectory: string) => string;
  wasmBinary?: ArrayBuffer;
}) => Promise<ModuleType>;
export default imageTools;
