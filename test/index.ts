import { optimizeImage } from '../dist/cjs';
import fs from 'node:fs';

const formats = ['webp', 'jpeg', 'png'] as const;

const main = async () => {
  fs.mkdirSync('./image_output', { recursive: true });
  const files = fs.readdirSync('./images');
  for (const file of files) {
    const data = fs.readFileSync(`./images/${file}`);
    for (const format of formats) {
      await optimizeImage({ image: data, width: 300, height: 200, quality: 100, format }).then(
        (encoded) => {
          console.log(encoded ? true : false, file, format);
          if (encoded) {
            const fileName = file.split('.')[0];
            fs.writeFileSync(`image_output/${fileName}.${format}`, encoded);
          }
        }
      );
    }
  }
};
main();
