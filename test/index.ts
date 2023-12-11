import { optimizeImage } from '../dist/cjs/index.js';
import fs from 'node:fs';

const main = async () => {
  const data = fs.readFileSync('./images/test01.jpg');
  const encoded = await optimizeImage({ image: data, width: 300, height: 200, quality: 100 });
  if (encoded) {
    fs.writeFileSync('./test2.webp', encoded);
  }
};
main();
