import { optimizeImage } from '../dist/cjs';
import fs from 'node:fs';

const formats = ['webp', 'jpeg', 'png'] as const;

const main = async () => {
  fs.mkdirSync('./image_output', { recursive: true });
  const files = fs.readdirSync('./images');
  for (const file of files) {
    const data = fs.readFileSync(`./images/${file}`);
    for (const format of formats) {
      await optimizeImage({ image: data, quality: 100, format }).then((encoded) => {
        console.log(encoded ? true : false, file, format);
        if (encoded) {
          const fileName = file.split('.')[0];
          fs.writeFileSync(`image_output/${fileName}.${format}`, encoded);
        }
      });
    }
  }
  for (let i = 0; i <= 8; i++) {
    const data = await fetch(
      `https://raw.githubusercontent.com/recurser/exif-orientation-examples/master/Landscape_${i}.jpg`
    ).then((res) => res.arrayBuffer());
    await optimizeImage({ image: data, quality: 100, format: 'jpeg' }).then((encoded) => {
      console.log(encoded ? true : false, `Landscape_${i}.jpg`, 'jpeg');
      if (encoded) {
        fs.writeFileSync(`image_output/Landscape_${i}.jpeg`, encoded);
      }
    });
  }
};
main();
