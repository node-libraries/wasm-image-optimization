import type { ModuleType } from "../esm/libImage.js";
import type { OptimizeParams } from "../types/index.js";

const result = (
  result: ReturnType<ModuleType["optimize"]> | undefined,
  releaseResult: () => void
) => {
  const r = result
    ? { ...result, data: Uint8Array.from(result.data) }
    : undefined;
  releaseResult();
  return r;
};
export const _optimizeImage = async ({
  image,
  width = 0,
  height = 0,
  quality = 100,
  format = "avif",
  speed = 6,
  filter = true,
  libImage,
}: OptimizeParams & {
  libImage: Promise<ModuleType>;
}) =>
  _optimizeImageExt({
    image,
    width,
    height,
    quality,
    format,
    speed,
    filter,
    libImage,
  }).then((r) => r?.data);

export const _optimizeImageExt = async ({
  image,
  width = 0,
  height = 0,
  quality = 100,
  format = "avif",
  speed = 6,
  filter = true,
  libImage,
}: OptimizeParams & {
  libImage: Promise<ModuleType>;
}) =>
  libImage.then(({ optimize, releaseResult }) =>
    result(
      optimize(image, width, height, quality, format, speed, filter),
      releaseResult
    )
  );
