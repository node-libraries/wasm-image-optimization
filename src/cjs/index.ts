import fs from "fs";
import path from "path";
import LibImage from "../workers/libImage";
import type { ModuleType } from "../esm/libImage";

const libImage = LibImage({
  wasmBinary: fs.readFileSync(
    path.resolve(__dirname, "../esm/libImage.wasm"),
  ) as never,
});

const result = (result?: ReturnType<ModuleType["optimize"]>) => {
  const r = result
    ? { ...result, data: Uint8Array.from(result.data) }
    : undefined;
  if (r) {
    libImage.then(({ releaseResult }) => releaseResult(r.index));
  }
  return r;
};
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
  optimizeImageExt({ image, width, height, quality, format }).then(
    (r) => r?.data,
  );

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
  libImage.then(({ optimize }) =>
    result(optimize(image, width, height, quality, format)),
  );
