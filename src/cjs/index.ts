import fs from "fs";
import path from "path";
import LibImage from "../workers/libImage";

const libImage = LibImage({
  wasmBinary: fs.readFileSync(path.resolve(__dirname, "../esm/libImage.wasm")),
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
  libImage.then(({ optimize }) =>
    optimize(image, width, height, quality, format)
  );
