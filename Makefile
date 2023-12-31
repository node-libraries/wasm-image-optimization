SHELL=/bin/bash
WORKDIR=work

TARGET_ESM = dist/esm/$(basename $(notdir src/libImage.cpp)).js
TARGET_WORKERS = dist/workers/$(basename $(notdir src/libImage.cpp)).js

CFLAGS = -O3 --bind -msimd128 \
  -Ilibwebp -Ilibwebp/src -Ilibavif/include -Ilibavif/third_party/libyuv/include -Ilibavif/ext/aom \
  -DAVIF_CODEC_AOM_DECODE -DAVIF_CODEC_AOM=LOCAL

WEBP_SOURCES := $(wildcard libwebp/src/dsp/*.c) \
                $(wildcard libwebp/src/enc/*.c) \
                $(wildcard libwebp/src/utils/*.c) \
                $(wildcard libwebp/src/dec/*.c) \
                $(wildcard libwebp/sharpyuv/*.c)
WEBP_OBJECTS := $(patsubst %.c, %.o, $(WEBP_SOURCES))

AVIF_SOURCES := libavif/src/alpha.c \
                libavif/src/avif.c \
                libavif/src/colr.c \
                libavif/src/colrconvert.c \
                libavif/src/diag.c \
                libavif/src/exif.c \
                libavif/src/io.c \
                libavif/src/mem.c \
                libavif/src/obu.c \
                libavif/src/rawdata.c \
                libavif/src/read.c \
                libavif/src/reformat.c \
                libavif/src/reformat_libsharpyuv.c \
                libavif/src/reformat_libyuv.c \
                libavif/src/scale.c \
                libavif/src/stream.c \
                libavif/src/utils.c \
                libavif/src/write.c \
                libavif/src/codec_aom.c \
                libavif/third_party/libyuv/source/scale.c \
                libavif/third_party/libyuv/source/scale_common.c \
                libavif/third_party/libyuv/source/scale_any.c \
                libavif/third_party/libyuv/source/row_common.c \
                libavif/third_party/libyuv/source/planar_functions.c
AVIF_OBJECTS := $(patsubst %.c, %.o, $(AVIF_SOURCES))

.PHONY: all esm workers

all: esm workers

$(WORKDIR):
	mkdir -p $(WORKDIR)

$(WORKDIR)/webp.a: $(WORKDIR) $(WEBP_OBJECTS)
	emar rcs $@ $(WEBP_OBJECTS)

$(WORKDIR)/avif.a: $(WORKDIR) $(AVIF_OBJECTS)
	emar rcs $@ $(AVIF_OBJECTS)

%.o: %.c
	emcc ${CFLAGS} -c $< -o $@

esm: $(TARGET_ESM)

$(TARGET_ESM): src/libImage.cpp $(WORKDIR)/webp.a $(WORKDIR)/avif.a libavif/ext/aom_build/libaom.a
	mkdir -p dist/esm
	emcc $(CFLAGS) \
	-s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s ENVIRONMENT=web -s DYNAMIC_EXECUTION=0 -s MODULARIZE=1 -s EXPORT_ES6=1 \
	-s USE_SDL=2 -s USE_SDL_IMAGE=2 \
	-s SDL2_IMAGE_FORMATS='["png","jpg","webp","svg","avif"]' \
	-o $(TARGET_ESM) \
	$^

workers: $(TARGET_WORKERS)

$(TARGET_WORKERS): src/libImage.cpp $(WORKDIR)/webp.a $(WORKDIR)/avif.a libavif/ext/aom_build/libaom.a
	mkdir -p dist/workers
	emcc $(CFLAGS) \
	-s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s ENVIRONMENT=web -s DYNAMIC_EXECUTION=0 -s MODULARIZE=1 \
	-s USE_SDL=2 -s USE_SDL_IMAGE=2 \
	-s SDL2_IMAGE_FORMATS='["png","jpg","webp","svg","avif"]' \
	-o $(TARGET_WORKERS) \
	$^
	rm dist/workers/libImage.wasm

clean:
	rm -rf $(WORKDIR)
	rm -rf dist/workers/libImage.*
	rm -rf dist/esm/libImage.*
