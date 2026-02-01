SHELL=/bin/bash
WORKDIR=work
DISTDIR=dist
ESMDIR=$(DISTDIR)/esm
WORKERSDIR=$(DISTDIR)/cjs
LIBDIR=libavif/ext
MAIN=src/cpp/libImage.cpp
LIBS=$(wildcard src/cpp/libs/*.cpp)

TARGET_ESM_BASE = $(notdir $(basename ${MAIN}))
TARGET_ESM = $(ESMDIR)/$(TARGET_ESM_BASE).js
TARGET_WORKERS = $(WORKERSDIR)/$(TARGET_ESM_BASE).js

CFLAGS = -O3 -msimd128 -sSTACK_SIZE=5MB \
        -Ilibwebp -Ilibwebp/src -Ilibavif/include -Ilibavif/third_party/libyuv/include -Ilibavif/ext/aom \
        -Ilibexif -Ilunasvg/include -Iavir\
        -DAVIF_CODEC_AOM_ENCODE -DAVIF_CODEC_AOM_DECODE -DAVIF_CODEC_AOM=LOCAL

CFLAGS_ASM = --bind \
             -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s ENVIRONMENT=web -s DYNAMIC_EXECUTION=0 -s MODULARIZE=1 \
             -s USE_SDL=3 -s USE_SDL_IMAGE=2 -s USE_SDL_GFX=2 \
             -s SDL2_IMAGE_FORMATS='["gif","png","jpg","webp","svg","avif"]'

WEBP_SOURCES := $(wildcard libwebp/src/dsp/*.c) \
                $(wildcard libwebp/src/enc/*.c) \
                $(wildcard libwebp/src/utils/*.c) \
                $(wildcard libwebp/src/dec/*.c) \
                $(wildcard libwebp/sharpyuv/*.c)
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
                libavif/src/gainmap.c \
                libavif/src/codec_aom.c \
								libavif/src/sampletransform.c \
                libavif/third_party/libyuv/source/scale.c \
                libavif/third_party/libyuv/source/scale_common.c \
                libavif/third_party/libyuv/source/scale_any.c \
                libavif/third_party/libyuv/source/row_common.c \
                libavif/third_party/libyuv/source/planar_functions.c

EXIF_SOURCES := $(wildcard libexif/libexif/*.c) \
                $(wildcard libexif/libexif/canon/*.c) \
                $(wildcard libexif/libexif/fuji/*.c) \
                $(wildcard libexif/libexif/olympus/*.c) \
                $(wildcard libexif/libexif/pentax/*.c) 

SVG_SOURCES := $(wildcard lunasvg/source/lunasvg/*.cpp)
OVG_SOURCES := $(wildcard lunasvg/source/plutovg/*.c)

WEBP_OBJECTS := $(WEBP_SOURCES:.c=.o)
AVIF_OBJECTS := $(AVIF_SOURCES:.c=.o)
EXIF_OBJECTS := $(EXIF_SOURCES:.c=.o)
SVG_OBJECTS := $(SVG_SOURCES:.cpp=.o)
OVG_OBJECTS := $(OVG_SOURCES:.c=.o)

.PHONY: all esm workers clean

all: esm workers

$(WEBP_OBJECTS) $(AVIF_OBJECTS)  $(OVG_OBJECTS): %.o: %.c | $(LIBDIR)/aom_build/libaom.a
	@emcc $(CFLAGS) -c $< -o $@

$(SVG_OBJECTS): %.o: %.cpp
	@emcc $(CFLAGS) -c $< -o $@

$(LIBDIR)/aom_build/libaom.a:
	@echo Building aom...
	@cd $(LIBDIR) && ./aom.cmd && mkdir -p aom_build && cd aom_build && \
	emcmake cmake ../aom \
    -DENABLE_CCACHE=1 \
    -DAOM_TARGET_CPU=generic \
    -DENABLE_DOCS=0 \
    -DENABLE_TESTS=0 \
    -DCONFIG_ACCOUNTING=1 \
    -DCONFIG_INSPECTION=1 \
    -DCONFIG_MULTITHREAD=0 \
    -DCONFIG_RUNTIME_CPU_DETECT=0 \
    -DCONFIG_WEBM_IO=0 \
    -DCMAKE_BUILD_TYPE=Release && \
	make aom

$(WORKDIR):
	@mkdir -p $(WORKDIR)

$(WORKDIR)/webp.a: $(WORKDIR) $(WEBP_OBJECTS)
	@emar rcs $@ $(WEBP_OBJECTS)

$(WORKDIR)/avif.a: $(WORKDIR) $(AVIF_OBJECTS)
	@emar rcs $@ $(AVIF_OBJECTS)

$(WORKDIR)/libexif.a: $(EXIF_SOURCES)
	@cd libexif && autoreconf -i && emconfigure ./configure && cd libexif && emmake make
	@emar rcs $@ $(EXIF_OBJECTS)

$(WORKDIR)/svg.a: $(WORKDIR) $(SVG_OBJECTS)
	@emar rcs $@ $(SVG_OBJECTS)
$(WORKDIR)/ovg.a: $(WORKDIR) $(OVG_OBJECTS)
	@emar rcs $@ $(OVG_OBJECTS)


$(ESMDIR) $(WORKERSDIR):
	@mkdir -p $@

esm: $(TARGET_ESM)

$(TARGET_ESM): ${MAIN} ${LIBS} $(WORKDIR)/webp.a $(WORKDIR)/avif.a $(WORKDIR)/libexif.a $(LIBDIR)/aom_build/libaom.a \
       $(WORKDIR)/svg.a $(WORKDIR)/ovg.a | $(ESMDIR)
	emcc $(CFLAGS) -o $@ $^ \
       $(CFLAGS_ASM)  -s EXPORT_ES6=1 -s SINGLE_FILE=1

workers: $(TARGET_WORKERS)

$(TARGET_WORKERS): ${MAIN} ${LIBS} $(WORKDIR)/webp.a $(WORKDIR)/avif.a $(WORKDIR)/libexif.a $(LIBDIR)/aom_build/libaom.a \
       $(WORKDIR)/svg.a $(WORKDIR)/ovg.a | $(WORKERSDIR)
	emcc $(CFLAGS) -o $@ $^ \
       $(CFLAGS_ASM)

clean:
	@echo Cleaning up...
	@rm -rf $(WORKDIR) $(LIBDIR)/aom_build $(ESMDIR) $(WORKERSDIR)
