export enum LogLevel {
  None = 0,
  Error = 1,
  Warning = 2,
  Info = 3,
  Debug = 4,
}

export interface ImageConverterModule {
  create_instance: () => any;
  destroy_instance: (inst: any) => void;
  load_image: (inst: any, data: Uint8Array) => boolean;
  encode: (
    inst: any,
    format: number,
    quality: number,
    speed: number,
    animation: boolean,
  ) => Uint8Array | null;
  crop: (inst: any, x: number, y: number, width: number, height: number) => boolean;
  resize: (inst: any, width: number, height: number, fit: number) => boolean;
  get_original_width: (inst: any) => number;
  get_original_height: (inst: any) => number;
  is_original_animation: (inst: any) => boolean;
  get_original_format: (inst: any) => string;
  get_width: (inst: any) => number;
  get_height: (inst: any) => number;
  is_animation: (inst: any) => boolean;
  get_format: (inst: any) => string;
  set_log_level: (level: number) => void;
  onLog?: (level: LogLevel, message: string) => void;
  logLevel: LogLevel;
}

export enum RenderFormat {
  None = -1,
  SVG = 0,
  PNG = 1,
  WebP = 2,
  PDF = 3,
  JPEG = 4,
  AVIF = 5,
  RAW = 6,
  ThumbHash = 7,
}

export enum FitMode {
  Contain = 0,
  Cover = 1,
  Fill = 2,
}

export type OptimizeParams = {
  image: Uint8Array | ArrayBuffer | string;
  crop?: { x: number; y: number; width: number; height: number };
  width?: number;
  height?: number;
  fit?: "contain" | "cover" | "fill";
  format?: "none" | "png" | "webp" | "jpeg" | "avif" | "raw" | "thumbhash";
  quality?: number;
  speed?: number;
  animation?: boolean;
};

export type OptimizeResult = {
  data: Uint8Array;
  originalWidth: number;
  originalHeight: number;
  originalAnimation: boolean;
  originalFormat: string;
  width: number;
  height: number;
  animation: boolean;
  format: string;
};

export abstract class ImageConverterBase {
  private factory: any;
  private modPromise?: Promise<ImageConverterModule>;
  private _onLog?: (level: LogLevel, message: string) => void;
  private _logLevel: LogLevel = LogLevel.None;

  protected constructor(factory: any) {
    this.factory = factory;
  }

  get onLog() {
    return this._onLog;
  }

  set onLog(val) {
    this._onLog = val;
    this.getModule().then((mod) => {
      mod.onLog = val;
    });
  }

  get logLevel() {
    return this._logLevel;
  }

  set logLevel(val) {
    this._logLevel = val;
    this.getModule().then((mod) => {
      mod.logLevel = val;
    });
  }

  private async getModule(): Promise<ImageConverterModule> {
    if (!this.modPromise) {
      this.modPromise = (async () => {
        let currentLogLevel = this._logLevel;
        let currentUserOnLog = this._onLog;

        const initialModule: any = {
          onLog: (level: LogLevel, message: string) => {
            if (
              this._logLevel !== LogLevel.None &&
              level <= this._logLevel &&
              this._onLog
            ) {
              this._onLog(level, message);
            }
          },
          print: (text: string) => {
            if (
              this._logLevel !== LogLevel.None &&
              LogLevel.Info <= this._logLevel &&
              this._onLog
            ) {
              this._onLog(LogLevel.Info, text);
            }
          },
          printErr: (text: string) => {
            if (
              this._logLevel !== LogLevel.None &&
              LogLevel.Error <= this._logLevel &&
              this._onLog
            ) {
              this._onLog(LogLevel.Error, text);
            }
          },
        };

        const mod: ImageConverterModule = (await this.factory(
          initialModule,
        )) as ImageConverterModule;

        mod.set_log_level(this._logLevel);

        // In modularized mode, the factory returns the module object.
        // EM_JS might be looking for functions on this object.
        (mod as any).onLog = initialModule.onLog;

        mod.logLevel = this._logLevel;

        const originalSetLogLevel = mod.set_log_level;
        mod.set_log_level = (level: number) => {
          this._logLevel = level;
          originalSetLogLevel(level);
        };

        let internalOnLog = mod.onLog;
        Object.defineProperty(mod, "onLog", {
          get: () => internalOnLog,
          set: (val) => {
            internalOnLog = val;
            this._onLog = val;
          },
        });

        let internalLogLevel = mod.logLevel;
        Object.defineProperty(mod, "logLevel", {
          get: () => internalLogLevel,
          set: (val) => {
            internalLogLevel = val;
            this._logLevel = val;
          },
        });

        return mod;
      })();
    }
    return this.modPromise;
  }

  async setLogLevel(level: LogLevel): Promise<void> {
    this.logLevel = level;
  }

  async optimizeImage(params: OptimizeParams): Promise<OptimizeResult> {
    const mod = await this.getModule();
    const inst = mod.create_instance();
    const {
      image: imageInput,
      width,
      height,
      fit = "contain",
      format,
      quality,
      speed = 6,
      animation = false,
      crop,
    } = params;
    const image =
      imageInput instanceof Uint8Array
        ? imageInput
        : typeof imageInput === "string"
          ? new TextEncoder().encode(imageInput)
          : new Uint8Array(imageInput);
    try {
      if (!mod.load_image(inst, image)) {
        throw new Error("Failed to load image");
      }

      const originalWidth = mod.get_original_width(inst);
      const originalHeight = mod.get_original_height(inst);
      const originalAnimation = mod.is_original_animation(inst);
      const originalFormat = mod.get_original_format(inst);

      let targetFormat = format || "webp";
      if (targetFormat !== "none" && animation && originalAnimation) {
        targetFormat = "webp";
      }

      if (crop) {
        mod.crop(inst, crop.x, crop.y, crop.width, crop.height);
      }

      if (targetFormat !== "none" && (width || height)) {
        const fitMap: Record<string, number> = {
          contain: FitMode.Contain,
          cover: FitMode.Cover,
          fill: FitMode.Fill,
        };
        const fMode = fitMap[fit] ?? FitMode.Contain;
        mod.resize(inst, width ?? 0, height ?? 0, fMode);
      }

      const formatMap: Record<string, number> = {
        none: RenderFormat.None,
        png: RenderFormat.PNG,
        webp: RenderFormat.WebP,
        jpeg: RenderFormat.JPEG,
        avif: RenderFormat.AVIF,
        raw: RenderFormat.RAW,
        thumbhash: RenderFormat.ThumbHash,
      };

      const f = formatMap[targetFormat] ?? RenderFormat.WebP;
      const result = mod.encode(inst, f, quality ?? 100, speed, animation);

      if (!result) {
        throw new Error("Failed to encode image");
      }

      return {
        data: new Uint8Array(result),
        originalWidth,
        originalHeight,
        originalAnimation,
        originalFormat,
        width: mod.get_width(inst),
        height: mod.get_height(inst),
        animation: mod.is_animation(inst),
        format: mod.get_format(inst),
      };
    } finally {
      mod.destroy_instance(inst);
    }
  }
}
