# wasm-image-optimization

# usage

```ts
import { optimizeImage } from 'wasm-image-optimization';

export interface Env {}

const handleRequest = async (
  request: Request,
  env: Env,
  ctx: ExecutionContext
): Promise<Response> => {
  const srcImage = await fetch(
    'https://raw.githubusercontent.com/SoraKumo001/zenn/master/images/0d107b03c58104/2022-04-14-17-03-07.png'
  ).then((res) => res.arrayBuffer());
  const image = await optimizeImage({
    image: srcImage,
    width: 200,
    height: 200,
  });
  return new Response(image, {
    headers: {
      'Content-Type': 'image/webp',
    },
  });
};

export default {
  fetch: handleRequest,
};
```
