import LibImage from './libImage.js';

const libImage = LibImage();

export const optimizeImage = async ({
  image,
  width = 0,
  height = 0,
  quality = 100,
}: {
  image: BufferSource;
  width?: number;
  height?: number;
  quality?: number;
}) => libImage.then(({ optimize }) => optimize(image, width, height, quality));
