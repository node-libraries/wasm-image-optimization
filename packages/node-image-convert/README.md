# node-image-convert

## Single-threaded

![](./document/convert_single.webp)

```ts
import { promises as fs } from "node:fs";
import { optimizeImage } from "wasm-image-optimization";

const formats = ["webp", "jpeg", "png", "avif"] as const;

const main = async () => {
  const title = "Single thread";
  console.time(title);
  await fs.mkdir("./image_output", { recursive: true });
  const files = await fs.readdir("./images");
  for (const file of files) {
    await fs.readFile(`./images/${file}`).then(async (image) => {
      console.log(
        `${file} ${Math.ceil(image.length / 1024).toLocaleString()}KB`
      );
      for (const format of formats) {
        const label = `[${file}] -> [${format}]`;
        console.time(label);
        await optimizeImage({
          image,
          quality: 80,
          format,
          width: 512,
        }).then((encoded) => {
          if (encoded) {
            console.timeLog(
              label,
              `${Math.ceil(encoded.length / 1024).toLocaleString()}KB`
            );
            const fileName = file.split(".")[0];
            fs.writeFile(`image_output/${fileName}.${format}`, encoded);
          }
        });
      }
    });
  }
  console.timeEnd(title);
};
main();
```

npm run dev

```txt
test01.jpg 115KB
[test01.jpg] -> [webp]: 108.31ms 39KB
[test01.jpg] -> [jpeg]: 23.907ms 54KB
[test01.jpg] -> [png]: 106.757ms 392KB
[test01.jpg] -> [avif]: 712.015ms 29KB
test02.jpg 267KB
[test02.jpg] -> [webp]: 90.049ms 62KB
[test02.jpg] -> [jpeg]: 23.598ms 70KB
[test02.jpg] -> [png]: 116.019ms 582KB
[test02.jpg] -> [avif]: 680.424ms 48KB
test03.avif 9KB
[test03.avif] -> [webp]: 56.15ms 13KB
[test03.avif] -> [jpeg]: 15.987ms 19KB
[test03.avif] -> [png]: 61.893ms 171KB
[test03.avif] -> [avif]: 182.797ms 9KB
test04.svg 1KB
[test04.svg] -> [webp]: 41.523ms 3KB
[test04.svg] -> [jpeg]: 6.936ms 9KB
[test04.svg] -> [png]: 15.259ms 9KB
[test04.svg] -> [avif]: 147.79ms 1KB
test05.svg 300KB
[test05.svg] -> [webp]: 74.179ms 12KB
[test05.svg] -> [jpeg]: 24.852ms 23KB
[test05.svg] -> [png]: 36.562ms 90KB
[test05.svg] -> [avif]: 716.63ms 9KB
Single thread: 3.281s
```

## Multi-threaded

![](./document/convert_multi.webp)

```ts
import { promises as fs } from "node:fs";
import {
  optimizeImage,
  close,
  setLimit,
} from "wasm-image-optimization/node-worker";

const formats = ["webp", "jpeg", "png", "avif"] as const;

setLimit(4); // set worker limit

const main = async () => {
  const title = "Multi thread";
  console.time(title);
  await fs.mkdir("./image_output", { recursive: true });
  const files = await fs.readdir("./images");
  const p = files.map(async (file) => {
    return fs.readFile(`./images/${file}`).then((image) => {
      console.log(
        `${file} ${Math.floor(image.length / 1024).toLocaleString()}KB`
      );
      const p = formats.map((format) => {
        const label = `[${file}] -> [${format}]`;
        console.time(label);
        return optimizeImage({
          image,
          quality: 100,
          format,
          width: 512,
        }).then((encoded) => {
          if (encoded) {
            console.timeLog(
              label,
              `${Math.floor(encoded.length / 1024).toLocaleString()}KB`
            );
            const fileName = file.split(".")[0];
            fs.writeFile(`image_output/${fileName}.${format}`, encoded);
          }
        });
      });
      return Promise.all(p);
    });
  });
  await Promise.all(p);
  close(); // close worker
  console.timeEnd(title);
};
main();
```

npm run dev:worker

```txt
test01.jpg 114KB
test04.svg 0KB
test03.avif 8KB
test05.svg 299KB
test02.jpg 266KB
[test01.jpg] -> [jpeg]: 244.211ms 211KB
[test04.svg] -> [webp]: 315.396ms 5KB
[test01.jpg] -> [png]: 324.986ms 391KB
[test04.svg] -> [jpeg]: 325.779ms 61KB
[test04.svg] -> [png]: 341.006ms 8KB
[test01.jpg] -> [webp]: 357.693ms 94KB
[test03.avif] -> [jpeg]: 425.789ms 65KB
[test03.avif] -> [webp]: 465.118ms 38KB
[test03.avif] -> [png]: 513.922ms 170KB
[test05.svg] -> [webp]: 596.433ms 30KB
[test05.svg] -> [jpeg]: 624.785ms 68KB
[test04.svg] -> [avif]: 627.652ms 1KB
[test05.svg] -> [png]: 664.596ms 89KB
[test03.avif] -> [avif]: 700.217ms 10KB
[test02.jpg] -> [jpeg]: 728.268ms 271KB
[test02.jpg] -> [webp]: 776.61ms 151KB
[test02.jpg] -> [png]: 850.041ms 581KB
[test01.jpg] -> [avif]: 1.045s 43KB
[test05.svg] -> [avif]: 1.434s 14KB
[test02.jpg] -> [avif]: 1.528s 74KB
Multi thread: 1.552s
```
