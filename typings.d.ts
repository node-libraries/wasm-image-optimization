declare module "*.wasm" {
  const content: never;
  export default content;
}

declare module "*.wasm?module" {
  const content: never;
  export default content;
}

declare module "*.wasm?init" {
  const content: (params: {
    imports?: WebAssembly.Imports;
  }) => Promise<WebAssembly.WebAssemblyInstantiatedSource>;
  export default content;
}

declare module "*.wasm?url" {
  const content: never;
  export default content;
}
