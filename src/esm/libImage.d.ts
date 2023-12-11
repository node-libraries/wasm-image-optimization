export declare type ModuleType = {
  optimize: (data: BufferSource, width: number, height: number, quality: number) => Uint8Array | null;
};
declare const imageTools: () => Promise<ModuleType>;
export default imageTools;
