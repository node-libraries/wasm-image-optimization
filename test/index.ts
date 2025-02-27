import fs from "node:fs";
import { optimizeImage, waitAll, close } from "../dist/cjs/node/cjs-worker";

const formats = ["webp", "jpeg", "png", "avif"] as const;

const main = async () => {
  fs.mkdirSync("./image_output", { recursive: true });
  const files = fs.readdirSync("./images");
  for (const file of files) {
    const data = fs.readFileSync(`./images/${file}`);
    for (const format of formats) {
      await optimizeImage({
        image: data,
        quality: 100,
        format,
        width: 500,
        speed: 6,
      }).then((encoded) => {
        console.log(encoded ? true : false, file, format);
        if (encoded) {
          const fileName = file.split(".")[0];
          fs.writeFileSync(`image_output/${fileName}.${format}`, encoded);
        }
      });
    }
  }
  const urls = [
    "https://raw.githubusercontent.com/AOMediaCodec/av1-avif/refs/heads/master/testFiles/Apple/multilayer_examples/animals_00_singlelayer.avif",
    "https://raw.githubusercontent.com/AOMediaCodec/av1-avif/refs/heads/master/testFiles/Microsoft/Chimera_10bit_cropped_to_1920x1008.avif",
  ];
  for (const url of urls) {
    const data = await fetch(url).then((res) => res.arrayBuffer());
    const u = new URL(url);
    const file = u.pathname.split("/").pop();
    if (!file) return;
    optimizeImage({
      image: data,
      quality: 100,
      format: "jpeg",
      width: 300,
      speed: 6,
    }).then(async () => {
      for (const format of formats) {
        optimizeImage({
          image: data,
          quality: 100,
          format,
          width: 100,
          height: 100,
        }).then((encoded) => {
          console.log(encoded ? true : false, file, format);
          if (encoded) {
            const fileName = file.split(".")[0];
            fs.writeFileSync(`image_output/${fileName}.${format}`, encoded);
          }
        });
      }
    });
  }

  for (let i = 0; i <= 8; i++) {
    const data = await fetch(
      `https://raw.githubusercontent.com/recurser/exif-orientation-examples/master/Landscape_${i}.jpg`
    ).then((res) => res.arrayBuffer());
    optimizeImage({
      image: data,
      quality: 100,
      format: "jpeg",
      width: 300,
    }).then((encoded) => {
      console.log(encoded ? true : false, `Landscape_${i}.jpg`, "jpeg");
      if (encoded) {
        fs.writeFileSync(`image_output/Landscape_${i}.jpeg`, encoded);
      }
    });
  }
  await waitAll();
  close();
};
main();
