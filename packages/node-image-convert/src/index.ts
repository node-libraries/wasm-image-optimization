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
        `${file} ${Math.ceil(image.length / 1024).toLocaleString()}KB`,
      );
      for (const format of formats) {
        const label = `[${file}] -> [${format}]`;
        console.time(label);
        await optimizeImage({
          image,
          quality: 80,
          format,
          width: 512,
        }).then(({ data }) => {
          if (data) {
            console.timeLog(
              label,
              `${Math.ceil(data.length / 1024).toLocaleString()}KB`,
            );
            const fileName = file.split(".")[0];
            fs.writeFile(`image_output/${fileName}.${format}`, data);
          }
        });
      }
    });
  }
  console.timeEnd(title);
};
main();
