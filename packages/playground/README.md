# next-image-convert

## Sample for converting image formats on Next.js.

https://next-image-convert.vercel.app/

```ts
"use client";
import React, { useEffect, useMemo, useRef, useState, type FC } from "react";
import { optimizeImage, setLimit } from "wasm-image-optimization";

setLimit(10); //Web Worker limit

const classNames = (...classNames: (string | undefined | false)[]) =>
  classNames.reduce(
    (a, b, index) => a + (b ? (index ? " " : "") + b : ""),
    ""
  ) as string | undefined;

const Time = () => {
  const [time, setTime] = useState(0);
  useEffect(() => {
    const handle = setInterval(() => setTime((v) => v + 1), 100);
    return () => clearInterval(handle);
  }, []);
  return (
    <div>
      {time} ← Test that the UI is working without being blocked during
      conversion.
    </div>
  );
};

const ImageInput: FC<{ onFiles: (files: File[]) => void }> = ({ onFiles }) => {
  const refInput = useRef<HTMLInputElement>(null);
  const [focus, setFocus] = useState(false);
  return (
    <>
      <div
        className={classNames(
          "w-64 h-32 border-dashed border flex justify-center items-center cursor-pointer select-none m-2 rounded-4xl p-4",
          focus && "outline outline-blue-400"
        )}
        onDragOver={(e) => {
          e.preventDefault();
          e.stopPropagation();
        }}
        onDragEnter={(e) => {
          e.preventDefault();
          e.stopPropagation();
        }}
        onDoubleClick={() => {
          refInput.current?.click();
        }}
        onClick={() => {
          refInput.current.focus();
        }}
        onDrop={(e) => {
          onFiles(Array.from(e.dataTransfer.files));
          e.preventDefault();
        }}
      >
        Drop here, copy and paste or double-click to select the file.
      </div>
      <input
        ref={refInput}
        className="absolute size-0"
        type="file"
        multiple
        accept=".jpg,.png,.gif,.svg,.avif,.webp"
        onFocus={() => setFocus(true)}
        onBlur={() => setFocus(false)}
        onPaste={(e) => {
          e.preventDefault();
          onFiles(Array.from(e.clipboardData.files));
        }}
        onChange={(e) => {
          e.preventDefault();
          onFiles(Array.from(e.currentTarget.files));
        }}
      />
    </>
  );
};

const formats = ["webp", "jpeg", "png", "avif"] as const;
const AsyncImage: FC<{ file: File; format: (typeof formats)[number] }> = ({
  file,
  format,
}) => {
  const [image, setImage] = useState<Uint8Array>();
  useEffect(() => {
    const convert = async () => {
      setImage(undefined);
      const image = await optimizeImage({
        image: await file.arrayBuffer(),
        format,
      });
      setImage(image);
    };
    convert();
  }, [file]);
  const src = useMemo(
    () =>
      image && URL.createObjectURL(new Blob([image], { type: "image/webp" })),
    [image]
  );
  const filename = file.name.replace(/\.\w+$/, `.${format}`);
  return (
    <div className="border border-gray-300 rounded-4 overflow-hidden relative w-64 h-64 grid">
      {src && image ? (
        <>
          <a download={filename} href={src}>
            <img
              className="flex-1 object-contain block overflow-hidden"
              src={src}
            />
          </a>
          <div className="bg-white/80 w-full z-10 text-right p-0 absolute bottom-0 font-bold">
            <div>{filename}</div>
            {Math.ceil(image.length / 1024).toLocaleString()}KB
          </div>
        </>
      ) : (
        <div className="m-auto animate-spin h-10 w-10 border-4 border-blue-600 rounded-full border-t-transparent" />
      )}
    </div>
  );
};

const Page = () => {
  const [images, setImages] = useState<File[]>([]);
  return (
    <div className="p-4">
      <div>
        <a
          className="text-blue-600 hover:underline"
          href="https://github.com/SoraKumo001/wasm-image-optimization-samples/tree/master/next-image-convert"
        >
          Source code
        </a>
      </div>
      Timer indicating that front-end processing has not stopped.
      <Time />
      <ImageInput onFiles={setImages} />
      <hr className="m-4" />
      <div className="flex flex-wrap gap-4">
        {images.flatMap((file, index) => (
          <div key={index} className="flex flex-wrap gap-4">
            {formats.map((format, index) => (
              <AsyncImage key={index} file={file} format={format} />
            ))}
          </div>
        ))}
      </div>
    </div>
  );
};
export default Page;
```
