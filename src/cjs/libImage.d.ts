export declare type ModuleType = {
  optimize: (
    data: BufferSource,
    width: number,
    height: number,
    quality: number
  ) => Uint8Array | null;
};

declare const imageTools: (options: {
  instantiateWasm?: (
    imports?: WebAssembly.Imports,
    receiver: (instance: WebAssembly.WebAssemblyInstantiatedSource) => Promise<unknown>
  ) => void;
}) => Promise<ModuleType>;
export default imageTools;
