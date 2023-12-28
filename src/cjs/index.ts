import LibImage from '../workers/libImage';
import path from 'path';
import fs from 'fs';

const libImage = LibImage({
  wasmBinary: fs.readFileSync(path.resolve(__dirname, '../esm/libImage.wasm')),
});

export const optimizeImage = async ({
  image,
  width = 0,
  height = 0,
  quality = 100,
  format = 'webp',
}: {
  image: BufferSource;
  width?: number;
  height?: number;
  quality?: number;
  format?: 'jpeg' | 'png' | 'webp';
}) => libImage.then(({ optimize }) => optimize(image, width, height, quality, format));
