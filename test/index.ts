import { promises as fs } from "node:fs";
import {
  optimizeImage,
  waitAll,
  waitReady,
  setLimit,
  close,
} from "../dist/cjs/node/cjs-worker";

const formats = ["webp", "jpeg", "png", "avif", "none"] as const;

setLimit(4);

const main = async () => {
  await fs.mkdir("./image_output", { recursive: true });
  const files = await fs.readdir("./images");
  const p = files.map((file) => {
    return fs.readFile(`./images/${file}`).then((image) => {
      console.log(
        `${file} ${Math.floor(image.length / 1024).toLocaleString()}KB`
      );
      const p = formats.map(async (format) => {
        await waitReady();
        const label = `[${file}] -> [${format}]`;
        console.time(label);
        return optimizeImage({
          image,
          quality: 100,
          format,
          width: 512,
        })
          .catch(() => undefined)
          .then((encoded) => {
            if (encoded) {
              console.timeLog(
                label,
                `${Math.floor(encoded.length / 1024).toLocaleString()}KB`
              );
              const fileName = file.split(".");
              const filePath =
                format === "none"
                  ? `image_output/${fileName[0]}_.${fileName[1]}`
                  : `image_output/${fileName[0]}.${format}`;
              fs.writeFile(filePath, encoded);
            }
          });
      });
      return Promise.all(p);
    });
  });
  await Promise.all(p);
  // const urls = [
  //   "https://raw.githubusercontent.com/AOMediaCodec/av1-avif/refs/heads/master/testFiles/Apple/multilayer_examples/animals_00_singlelayer.avif",
  //   "https://raw.githubusercontent.com/AOMediaCodec/av1-avif/refs/heads/master/testFiles/Microsoft/Chimera_10bit_cropped_to_1920x1008.avif",
  // ];
  // for (const url of urls) {
  //   const data = await fetch(url).then((res) => res.arrayBuffer());
  //   const u = new URL(url);
  //   const file = u.pathname.split("/").pop();
  //   if (!file) return;
  //   optimizeImage({
  //     image: data,
  //     quality: 100,
  //     format: "jpeg",
  //     width: 300,
  //     speed: 6,
  //   }).then(async () => {
  //     for (const format of formats) {
  //       optimizeImage({
  //         image: data,
  //         quality: 100,
  //         format,
  //         width: 100,
  //         height: 100,
  //       }).then((encoded) => {
  //         console.log(encoded ? true : false, file, format);
  //         if (encoded) {
  //           const fileName = file.split(".");
  //           const filePath =
  //             format === "none"
  //               ? `image_output/${fileName[0]}_.${fileName[1]}`
  //               : `image_output/${fileName[0]}.${format}`;
  //           fs.writeFile(filePath, encoded);
  //         }
  //       });
  //     }
  //   });
  // }

  // for (let i = 0; i <= 8; i++) {
  //   const data = await fetch(
  //     `https://raw.githubusercontent.com/recurser/exif-orientation-examples/master/Landscape_${i}.jpg`
  //   ).then((res) => res.arrayBuffer());
  //   optimizeImage({
  //     image: data,
  //     quality: 100,
  //     format: "jpeg",
  //     width: 300,
  //   }).then((encoded) => {
  //     console.log(encoded ? true : false, `Landscape_${i}.jpg`, "jpeg");
  //     if (encoded) {
  //       fs.writeFile(`image_output/Landscape_${i}.jpeg`, encoded);
  //     }
  //   });
  // }
  // await waitAll();
  close();
};
main();
