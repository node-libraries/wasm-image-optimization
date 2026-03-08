import { ImageConverterBase } from "./core.js";

export * from "./core.js";

export class ImageConverter extends ImageConverterBase {
  static async create(createImageConverterModuleFunc: any): Promise<ImageConverter> {
    return new ImageConverter(createImageConverterModuleFunc);
  }
}
