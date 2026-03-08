# cloudflare-ogp

Please use satori@0.15.2, later versions do not work with Cloudflare.

## usage

### parameters

| parameter | description                                                                                                                              |
| --------- | ---------------------------------------------------------------------------------------------------------------------------------------- |
| title     | The title of the page                                                                                                                    |
| name      | The name of the person or organization                                                                                                   |
| image     | URL of the image to be used as the Open Graph image <br> (WebP and AVIF images are automatically converted to PNG format for processing) |

### example

- Local test  
  http://127.0.0.1:8787/?title=TITLE_TEXT&name=NAME_TEXT&image=https://raw.githubusercontent.com/SoraKumo001/cloudflare-ogp/refs/heads/master/sample/image.jpg

## customization

You can customize what you see by editing `src/index.tsx`
