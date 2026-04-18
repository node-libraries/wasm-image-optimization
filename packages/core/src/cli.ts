#!/usr/bin/env node
import { program } from "commander";
import fs from "node:fs/promises";
import path from "node:path";
import { ImageConverter } from "./single.js";

// main() execution moved to top level
program
  .name("wasm-image-optimization")
  .description(
    "High-performance Image to Image converter using WebAssembly (Skia)",
  )
  .version("1.0.0")
  .argument("<input>", "input file path")
  .option("-o, --output <output>", "output file path")
  .option("-w, --width <number>", "resize width", parseInt)
  .option("-h, --height <number>", "resize height", parseInt)
  .option(
    "-f, --format <format>",
    "output format (none, png, webp, jpeg, avif, raw, thumbhash)",
    "webp",
  )
  .option("-q, --quality <number>", "output quality (0-100)", parseFloat, 85)
  .option(
    "-s, --speed <number>",
    "encoding speed (0-10, for AVIF)",
    parseInt,
    6,
  )
  .option("-a, --animation", "maintain animation frames", false)
  .option("--crop <x,y,w,h>", "crop region (format: x,y,width,height)", (val) => {
    const parts = val.split(",").map(Number);
    if (parts.length !== 4 || parts.some(isNaN)) {
      throw new Error("Invalid crop format. Use x,y,width,height");
    }
    return { x: parts[0], y: parts[1], width: parts[2], height: parts[3] };
  })
  .action(async (input, options) => {
    try {
      const inputData = await fs.readFile(input);
      const converter = await ImageConverter.create();

      console.log(`Converting ${input}...`);

      const result = await converter.optimizeImage({
        image: inputData,
        crop: options.crop,
        width: options.width,
        height: options.height,
        format: options.format as any,
        quality: options.quality,
        speed: options.speed,
        animation: options.animation,
      });

      let outputPath = options.output;
      if (!outputPath) {
        const ext = options.format || "webp";
        const parsed = path.parse(input);
        outputPath = path.join(parsed.dir, `${parsed.name}.${ext}`);
      }

      await fs.writeFile(outputPath, result.data);
      console.log(
        `Successfully converted to ${outputPath} (${result.width}x${result.height})`,
      );
    } catch (err: any) {
      console.error(`Error: ${err.message}`);
      process.exit(1);
    }
  });

program.parseAsync(process.argv);
