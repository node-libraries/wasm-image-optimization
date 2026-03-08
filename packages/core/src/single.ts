import {
  ImageConverter as ImageConverterBase,
  OptimizeParams,
  OptimizeResult,
} from "./index.js";

export * from "./core.js";

/**
 * Single-file specialized wrapper for Satoru.
 * Uses the satoru-single.js artifact with embedded WASM.
 */
export class ImageConverter extends ImageConverterBase {
  // Use protected constructor from base
  private constructor(factory: any) {
    super(factory);
  }

  /**
   * Create Satoru instance with embedded WASM.
   */
  static async create(): Promise<ImageConverter> {
    const { default: createSatoruModuleSingle } =
      // @ts-ignore
      await import("../dist/wasm-image-optimization-single.js");
    return new ImageConverter(createSatoruModuleSingle);
  }
}

let instance: ImageConverter | null = null;

export async function optimizeImage(
  params: OptimizeParams,
): Promise<OptimizeResult> {
  if (!instance) {
    instance = await ImageConverter.create();
  }
  return instance.optimizeImage(params);
}
