export type OptimizeResult = {
  data: Uint8Array;
  originalWidth: number;
  originalHeight: number;
  width: number;
  height: number;
};

export type OptimizeParams = {
  image: BufferSource | Buffer | string; // The input image data
  width?: number; // The desired output width (optional)
  height?: number; // The desired output height (optional)
  quality?: number; // The desired output quality (0-100, optional)
  format?: "jpeg" | "png" | "webp" | "avif" | "thumbhash" | "raw" | "none"; // The desired output format (optional)
  speed?: number; // The desired speed (0-10, Slow-Fast, Optional)
  filter?: boolean; // resize filter (default true)
};
