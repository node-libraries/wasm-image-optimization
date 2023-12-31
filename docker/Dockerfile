FROM emscripten/emsdk
WORKDIR /app
RUN git clone https://github.com/webmproject/libwebp &&\
    git clone https://github.com/AOMediaCodec/libavif &&\
    ln -s /app/libwebp/src/webp /emsdk/upstream/lib/clang/18/include/webp &&\
    ln -s /app/libavif/include/avif /emsdk/upstream/lib/clang/18/include/avif &&\
    cd libavif/ext && ./aom.cmd && mkdir aom_build && cd aom_build && \
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
    -DCMAKE_BUILD_TYPE=Release &&\
    make aom
