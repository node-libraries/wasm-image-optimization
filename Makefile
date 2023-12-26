SHELL=/bin/bash
target: esm workers
esm: src/libImage.cpp
	mkdir -p dist/esm
	emcc -O3 --bind -msimd128 \
    -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s ENVIRONMENT=web -s DYNAMIC_EXECUTION=0 -s MODULARIZE=1 -s EXPORT_ES6=1 \
    -s USE_SDL=2 -s USE_SDL_IMAGE=2 -DAVIF_CODEC_AOM=LOCAL \
    -s SDL2_IMAGE_FORMATS='["png","jpg","webp","svg","avif"]' \
    -I libwebp -I libwebp/src libwebp/src/{dsp,enc,utils,dec}/*.c libwebp/sharpyuv/*.c \
    -I libavif/include -I libavif/third_party/libyuv/include \
    libavif/src/{alpha,avif,colr,colrconvert,diag,exif,io,mem,obu,rawdata,read,reformat,reformat_libsharpyuv,reformat_libyuv,scale,stream,utils,write,codec_aom}.c \
    libavif/third_party/libyuv/source/{scale,scale_common,scale_any,row_common,planar_functions}.c \
    -I libavif/ext/aom -DAVIF_CODEC_AOM_DECODE -DAVIF_CODEC_AOM_ENCODE libavif/ext/aom_build/libaom.a \
    -o dist/esm/$(basename $(<F)).js $< 
    
workers: src/libImage.cpp
	mkdir -p dist/workers
	emcc -O3 --bind -msimd128 \
    -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s ENVIRONMENT=web -s DYNAMIC_EXECUTION=0 -s MODULARIZE=1 \
    -s USE_SDL=2 -s USE_SDL_IMAGE=2 -DAVIF_CODEC_AOM=LOCAL \
    -s SDL2_IMAGE_FORMATS='["png","jpg","webp","svg","avif"]' \
    -I libwebp -I libwebp/src libwebp/src/{dsp,enc,utils,dec}/*.c libwebp/sharpyuv/*.c \
    -I libavif/include -I libavif/third_party/libyuv/include \
    libavif/src/{alpha,avif,colr,colrconvert,diag,exif,io,mem,obu,rawdata,read,reformat,reformat_libsharpyuv,reformat_libyuv,scale,stream,utils,write,codec_aom}.c \
    libavif/third_party/libyuv/source/{scale,scale_common,scale_any,row_common,planar_functions}.c \
    -I libavif/ext/aom -DAVIF_CODEC_AOM_DECODE -DAVIF_CODEC_AOM_ENCODE libavif/ext/aom_build/libaom.a \
    -o dist/workers/$(basename $(<F)).js $< 
	rm dist/workers/$(basename $(<F)).wasm
