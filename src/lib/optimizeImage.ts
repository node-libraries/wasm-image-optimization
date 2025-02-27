import { type ModuleType } from "../esm/libImage.js";

export type OptimizeParams = {
  image: BufferSource | string; // The input image data
  width?: number; // The desired output width (optional)
  height?: number; // The desired output height (optional)
  quality?: number; // The desired output quality (0-100, optional)
  format?: "jpeg" | "png" | "webp" | "avif"; // The desired output format (optional)
  speed?: number; // The desired speed (0-10, Slow-Fast, Optional)
};

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
    libImage,
  }).then((r) => r?.data);

export const _optimizeImageExt = async ({
  image,
  width = 0,
  height = 0,
  quality = 100,
  format = "avif",
  speed = 6,
  libImage,
}: OptimizeParams & {
  libImage: Promise<ModuleType>;
}) =>
  libImage.then(({ optimize, releaseResult }) =>
    result(
      optimize(image, width, height, quality, format, speed),
      releaseResult
    )
  );
