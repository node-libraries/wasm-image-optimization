FROM emscripten/emsdk
WORKDIR /app
RUN  apt-get update && apt-get install -y dh-autoreconf ninja-build yasm &&\
    git clone https://github.com/webmproject/libwebp &&\
    git clone https://github.com/AOMediaCodec/libavif &&\
    git clone https://github.com/libexif/libexif &&\
    git clone https://github.com/SoraKumo001/lunasvg &&\
    git clone https://github.com/avaneev/avir &&\
    ln -s /app/libavif/include/avif /emsdk/upstream/lib/clang/23/include/avif &&\
    ln -s /app/libwebp/src/webp /emsdk/upstream/lib/clang/23/include/webp
