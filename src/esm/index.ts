import LibImage from './libImage.js';

const libImage = LibImage();

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
