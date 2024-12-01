import fs from "fs";
import path from "path";
import LibImage from "../workers/libImage";

const libImage = LibImage({
  wasmBinary: fs.readFileSync(
    path.resolve(__dirname, "../esm/libImage.wasm")
  ) as never,
});

export const optimizeImage = async ({
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
