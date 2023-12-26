import { optimizeImage } from '../dist/cjs';
import fs from 'node:fs';

const main = async () => {
  fs.mkdirSync('./image_output', { recursive: true });
  const files = fs.readdirSync('./images');
  files.forEach((file) => {
    const data = fs.readFileSync(`./images/${file}`);
    optimizeImage({ image: data, width: 300, height: 200, quality: 100 }).then((encoded) => {
      console.log(encoded ? true : false, file);
      if (encoded) {
        const fileName = file.split('.')[0];
        fs.writeFileSync(`image_output/${fileName}.webp`, encoded);
      }
    });
  });
};
main();
