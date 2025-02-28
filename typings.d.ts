declare module "*.wasm" {
  const content: any;
  export default content;
}

declare module "*.wasm?module" {
  const content: any;
  export default content;
}

declare module "*.wasm?init" {
  const content: (params: {
    imports?: WebAssembly.Imports;
  }) => Promise<WebAssembly.WebAssemblyInstantiatedSource>;
  export default content;
}

declare module "*.wasm?url" {
  const content: any;
  export default content;
}
