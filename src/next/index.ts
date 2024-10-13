import WASM from "../esm/libImage.wasm?module";
import LibImage from "../workers//libImage.js";

const libImage = LibImage({
  instantiateWasm: async (imports, receiver) => {
    receiver(await WebAssembly.instantiate(WASM, imports));
  },
});

export const optimizeImage = async ({
  image,
  width = 0,
  height = 0,
  quality = 100,
  format = "webp",
}: {
  image: BufferSource;
  width?: number;
  height?: number;
  quality?: number;
  format?: "jpeg" | "png" | "webp" | "avif";
}) =>
  libImage.then(({ optimize }) => {
    const result = optimize(image, width, height, quality, format);
    return result?.data;
  });

export const optimizeImageExt = async ({
  image,
  width = 0,
  height = 0,
  quality = 100,
  format = "avif",
}: {
  image: BufferSource;
  width?: number;
  height?: number;
  quality?: number;
  format?: "jpeg" | "png" | "webp" | "avif";
}) =>
  libImage.then(({ optimize }) => {
    return optimize(image, width, height, quality, format);
  });
