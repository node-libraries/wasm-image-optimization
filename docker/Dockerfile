FROM emscripten/emsdk
WORKDIR /app
RUN git clone https://github.com/webmproject/libwebp &&\
    git clone https://github.com/AOMediaCodec/libavif &&\
    ln -s /app/libwebp/src/webp /emsdk/upstream/lib/clang/18/include/webp &&\
    ln -s /app/libavif/include/avif /emsdk/upstream/lib/clang/18/include/avif
