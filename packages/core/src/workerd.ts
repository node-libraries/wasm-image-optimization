// @ts-ignore
import createModule from "../dist/wasm-image-optimization.js";

import {
  ImageConverter as ImageConverterBase,
  OptimizeParams,
  OptimizeResult,
} from "./index.js";

export * from "./core.js";

// @ts-ignore
import satoruWasm from "../dist/satoru.wasm";

/**
 * Cloudflare Workers (workerd) specialized wrapper for Satoru.
 * Handles the specific WASM instantiation requirements for the environment.
 */
export class ImageConverter extends ImageConverterBase {
  // Use protected constructor from base
  private constructor(factory: any) {
    super(factory);
  }

  /**
   * Create Satoru instance for Cloudflare Workers.
   * @param wasm The compiled WASM module (defaults to the bundled one)
   */
  static async create(
    wasm: WebAssembly.Module = satoruWasm,
  ): Promise<ImageConverter> {
    const factory = async (o: any) =>
      createModule({
        ...o,
        instantiateWasm: (imports: any, successCallback: any) => {
          // Cloudflare Workers requires using the pre-compiled WebAssembly.Module
          WebAssembly.instantiate(wasm, imports)
            .then((instance) => {
              successCallback(instance, wasm);
            })
            .catch((e) => {
              console.error(
                "wasm-image-optimization [workerd]: Wasm instantiation failed:",
                e,
              );
            });
          return {}; // Return empty object as emscripten expects
        },
      });

    return ImageConverterBase.create(factory) as Promise<ImageConverter>;
  }
}

let instance: ImageConverter | null = null;

/**
 * High-level convert function for Cloudflare Workers.
 * Automatically creates and reuses a Satoru instance.
 */
export async function optimizeImage(
  params: OptimizeParams,
): Promise<OptimizeResult> {
  if (!instance) {
    instance = await ImageConverter.create();
  }
  return instance.optimizeImage(params);
}
