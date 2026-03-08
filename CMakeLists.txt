cmake_minimum_required(VERSION 3.14)

# --- Toolchain setup ---
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
set(VCPKG_TARGET_TRIPLET "wasm32-emscripten-wasm-eh" CACHE STRING "")
set(VCPKG_OVERLAY_TRIPLETS "${CMAKE_CURRENT_SOURCE_DIR}/triplets" CACHE STRING "")
set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "$ENV{EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake" CACHE STRING "")

project(image_converter_wasm)
set(CMAKE_CXX_STANDARD 20)

# --- Enable ccache ---
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    message(STATUS "ccache enabled")
endif()

# --- Directories ---
set(LIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/libs")
set(SKIA_CONFIG_DIR "${LIBS_DIR}/skia")

include(FetchContent)

# --- Skia ---
FetchContent_Declare(
  skia
  GIT_REPOSITORY https://github.com/google/skia.git
  GIT_TAG        main
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(skia)

# --- libavif ---
set(VCPKG_INSTALLED_DIR "${CMAKE_BINARY_DIR}/vcpkg_installed/wasm32-emscripten-wasm-eh")

# --- libaom ---
find_package(Perl REQUIRED)
set(PERL_EXECUTABLE ${PERL_EXECUTABLE} CACHE FILEPATH "" FORCE)
set(AOM_TARGET_CPU "generic" CACHE STRING "" FORCE)
set(ENABLE_DOCS OFF CACHE BOOL "" FORCE)
set(ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ENABLE_TOOLS OFF CACHE BOOL "" FORCE)
set(CONFIG_RUNTIME_CPU_DETECT 0 CACHE BOOL "" FORCE)
set(CONFIG_MULTITHREAD 0 CACHE BOOL "" FORCE)
set(CONFIG_WEBM_IO 0 CACHE BOOL "" FORCE)

FetchContent_Declare(
  libaom
  GIT_REPOSITORY https://aomedia.googlesource.com/aom
  GIT_TAG        v3.11.0
  GIT_SHALLOW    TRUE
)
FetchContent_GetProperties(libaom)
if(NOT libaom_POPULATED)
    FetchContent_Populate(libaom)
    
    # Save current flags and suppress all warnings for libaom
    set(OLD_CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
    set(OLD_CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
    
    add_subdirectory(${libaom_SOURCE_DIR} ${libaom_BINARY_DIR} EXCLUDE_FROM_ALL)
    
    # Restore flags
    set(CMAKE_C_FLAGS ${OLD_CMAKE_C_FLAGS})
    set(CMAKE_CXX_FLAGS ${OLD_CMAKE_CXX_FLAGS})
endif()

# --- libavif ---
set(AVIF_CODEC_AOM SYSTEM CACHE STRING "" FORCE)
set(AVIF_CODEC_AOM_DECODE ON CACHE BOOL "" FORCE)
set(AVIF_CODEC_AOM_ENCODE ON CACHE BOOL "" FORCE)
set(AVIF_LIBYUV OFF CACHE BOOL "" FORCE)
set(AVIF_BUILD_APPS OFF CACHE BOOL "" FORCE)
set(AVIF_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(AVIF_ENABLE_WASM ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  libavif
  GIT_REPOSITORY https://github.com/AOMediaCodec/libavif.git
  GIT_TAG        v1.1.1
)

set(CMAKE_SKIP_INSTALL_RULES ON)

# Save current flags and suppress all warnings for libavif
set(OLD_CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
set(OLD_CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")

FetchContent_MakeAvailable(libavif)

# Restore flags
set(CMAKE_C_FLAGS ${OLD_CMAKE_C_FLAGS})
set(CMAKE_CXX_FLAGS ${OLD_CMAKE_CXX_FLAGS})

set(CMAKE_SKIP_INSTALL_RULES OFF)

# Manually fix libavif missing include directories for aom
if(TARGET avif_obj)
    target_include_directories(avif_obj PRIVATE 
        ${libaom_SOURCE_DIR}
        ${libaom_BINARY_DIR}
        ${libaom_SOURCE_DIR}/aom
    )
endif()

# --- wuffs ---
set(WUFFS_RELEASE_DIR "${CMAKE_BINARY_DIR}/wuffs")
if(NOT EXISTS "${WUFFS_RELEASE_DIR}/wuffs-v0.3.c")
    file(MAKE_DIRECTORY "${WUFFS_RELEASE_DIR}")
    message(STATUS "Downloading wuffs-v0.3.c...")
    file(DOWNLOAD https://raw.githubusercontent.com/google/wuffs/main/release/c/wuffs-v0.3.c
         "${WUFFS_RELEASE_DIR}/wuffs-v0.3.c")
endif()

# --- Dependencies ---
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)
find_package(WebP REQUIRED)
find_package(EXPAT REQUIRED)

set(IMAGE_CONVERTER_LIBS
    PNG::PNG
    JPEG::JPEG
    WebP::webp
    WebP::webpdecoder
    WebP::webpdemux
    WebP::libwebpmux
    WebP::sharpyuv
    EXPAT::EXPAT
    aom
    avif
    ZLIB::ZLIB
)

set(COMMON_COMPILE_OPTIONS
    -Oz
    -g0
    -fexceptions
    -msimd128
    -flto=thin
    -Wno-unknown-warning-option
)
set(COMMON_LINK_OPTIONS
    "--bind"
    "-O3"
    "-g0"
    -fexceptions
    -msimd128
    "-flto=thin"
    "-sWASM_BIGINT=1"
    "-sSUPPORT_LONGJMP=emscripten"
    "-sDISABLE_EXCEPTION_CATCHING=0"
    "-sDYNAMIC_EXECUTION=0"
    "-sASSERTIONS=0"
    "-sMODULARIZE=1"
    "-sEXPORT_ES6=1"
    "-sEXPORT_NAME='createImageConverterModule'"
    "-sENVIRONMENT=web,worker"
    "-sFILESYSTEM=0"
    "-sERROR_ON_UNDEFINED_SYMBOLS=0"
    "-sWARN_ON_UNDEFINED_SYMBOLS=1"
    "-sALLOW_MEMORY_GROWTH=1"
)

# --- Target: skia_lib ---
add_library(skia_lib STATIC)
file(GLOB SKIA_CORE_SRC
    "${skia_SOURCE_DIR}/src/core/*.cpp"
    "${skia_SOURCE_DIR}/src/base/*.cpp"
    "${skia_SOURCE_DIR}/src/utils/*.cpp"
    "${skia_SOURCE_DIR}/src/ports/SkOSFile_posix.cpp"
    "${skia_SOURCE_DIR}/src/ports/SkOSFile_stdio.cpp"
    "${skia_SOURCE_DIR}/src/ports/SkTime_Unix.cpp"
    "${skia_SOURCE_DIR}/src/ports/SkMemory_malloc.cpp"
    "${skia_SOURCE_DIR}/src/ports/SkDiscardableMemory_none.cpp"
    "${skia_SOURCE_DIR}/src/ports/SkFontMgr_custom.cpp"
    "${skia_SOURCE_DIR}/src/ports/SkFontMgr_custom_empty.cpp"
    "${skia_SOURCE_DIR}/src/ports/SkLog_stdio.cpp"
    "${skia_SOURCE_DIR}/src/ports/SkGlobalInitialization_default.cpp"
    "${skia_SOURCE_DIR}/src/core/SkRuntimeBlender.cpp"
    "${skia_SOURCE_DIR}/src/image/*.cpp"
    "${skia_SOURCE_DIR}/src/shaders/*.cpp"
    "${skia_SOURCE_DIR}/src/shaders/gradients/*.cpp"
    "${skia_SOURCE_DIR}/src/pathops/*.cpp"
    "${skia_SOURCE_DIR}/src/effects/*.cpp"
    "${skia_SOURCE_DIR}/src/effects/colorfilters/*.cpp"
    "${skia_SOURCE_DIR}/src/effects/imagefilters/*.cpp"
    "${skia_SOURCE_DIR}/src/svg/*.cpp"
    "${skia_SOURCE_DIR}/src/xml/*.cpp"
    "${skia_SOURCE_DIR}/src/text/*.cpp"
    "${skia_SOURCE_DIR}/src/sksl/*.cpp"
    "${skia_SOURCE_DIR}/src/sksl/analysis/*.cpp"
    "${skia_SOURCE_DIR}/src/sksl/codegen/SkSLRasterPipelineBuilder.cpp"
    "${skia_SOURCE_DIR}/src/sksl/codegen/SkSLRasterPipelineCodeGenerator.cpp"
    "${skia_SOURCE_DIR}/src/sksl/ir/*.cpp"
    "${skia_SOURCE_DIR}/src/sksl/transform/*.cpp"
    "${skia_SOURCE_DIR}/src/sksl/tracing/*.cpp"
    "${skia_SOURCE_DIR}/modules/skcms/skcms.cc"
    "${skia_SOURCE_DIR}/modules/skcms/src/skcms_TransformBaseline.cc"
    "${skia_SOURCE_DIR}/src/encode/SkEncoder.cpp"
    "${skia_SOURCE_DIR}/src/encode/SkPngEncoderImpl.cpp"
    "${skia_SOURCE_DIR}/src/encode/SkPngEncoderBase.cpp"
    "${skia_SOURCE_DIR}/src/encode/SkJpegEncoderImpl.cpp"
    "${skia_SOURCE_DIR}/src/encode/SkJPEGWriteUtility.cpp"
    "${skia_SOURCE_DIR}/src/encode/SkWebpEncoderImpl.cpp"
    "${skia_SOURCE_DIR}/src/encode/SkICC.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkCodec.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkCodecColorProfile.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkCodecImageGenerator.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkImageGenerator_FromEncoded.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkEncodedInfo.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkPngCodec.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkPngCodecBase.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkPngCompositeChunkReader.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkExif.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkTiffUtility.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkGainmapInfo.cpp"
    "${skia_SOURCE_DIR}/src/utils/SkOTUtils.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkBmpBaseCodec.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkBmpCodec.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkBmpMaskCodec.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkBmpRLECodec.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkBmpStandardCodec.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkIcoCodec.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkParseEncodedOrigin.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkMaskSwizzler.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkSwizzler.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkSampler.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkColorPalette.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkColorTable.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkPixmapUtils.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkHdrMetadata.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkWuffsCodec.cpp"
    "src/cpp/libs/skia/wuffs_implementation.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkJpegCodec.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkJpegDecoderMgr.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkJpegMetadataDecoderImpl.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkJpegMultiPicture.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkJpegSegmentScan.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkJpegSourceMgr.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkJpegUtility.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkJpegXmp.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkWebpCodec.cpp"
    "${skia_SOURCE_DIR}/src/codec/SkAvifCodec.cpp"
    "${skia_SOURCE_DIR}/modules/svg/src/*.cpp"
    "${skia_SOURCE_DIR}/modules/skresources/src/*.cpp"
    "${skia_SOURCE_DIR}/modules/skshaper/src/SkShaper.cpp"
    "${skia_SOURCE_DIR}/modules/skshaper/src/SkShaper_primitive.cpp"
    "${skia_SOURCE_DIR}/modules/skshaper/src/SkShaper_factory.cpp"
)
list(FILTER SKIA_CORE_SRC EXCLUDE REGEX ".*_win.cpp|.*_mac.cpp|.*_linux.cpp|.*_android.cpp|.*_ios.cpp")
list(FILTER SKIA_CORE_SRC EXCLUDE REGEX ".*SkGetExecutablePath.*|.*SkThread.*|.*SkSemaphore.*|.*SkMesh.*|.*SkGPU.*|.*SkGpu.*|.*SkGraphite.*|.*SkDocument_PDF_None.cpp|.*src/gpu/.*|.*src/graphite/.*")

target_sources(skia_lib PRIVATE ${SKIA_CORE_SRC})
target_include_directories(skia_lib PUBLIC
    "${SKIA_CONFIG_DIR}"
    "${skia_SOURCE_DIR}"
    "${WUFFS_RELEASE_DIR}"
    "${skia_SOURCE_DIR}/modules/svg/include"
    "${skia_SOURCE_DIR}/modules/skresources/include"
)
target_compile_definitions(skia_lib PUBLIC SK_USER_CONFIG_HEADER="SkUserConfig.h" SK_ENABLE_SVG SK_DISABLE_FILESYSTEM SKCMS_PORTABLE)
target_compile_options(skia_lib PRIVATE ${COMMON_COMPILE_OPTIONS} -w)
target_link_libraries(skia_lib PUBLIC ${IMAGE_CONVERTER_LIBS})

# --- Target: image_converter_core ---
add_library(image_converter_core OBJECT)
target_sources(image_converter_core PRIVATE
    src/cpp/main.cpp
    src/cpp/api/image_converter_api.cpp
    src/cpp/core/image_converter_context.cpp
    src/cpp/utils/skia_utils.cpp
    src/cpp/utils/skia_stubs.cpp
    src/cpp/utils/thumbhash.cpp
)
target_include_directories(image_converter_core PRIVATE
    "src/cpp"
)
target_link_libraries(image_converter_core PUBLIC skia_lib)
target_compile_options(image_converter_core PRIVATE ${COMMON_COMPILE_OPTIONS})
target_compile_definitions(image_converter_core PRIVATE SK_USER_CONFIG_HEADER="SkUserConfig.h")

# --- Executables ---
add_executable(image_converter $<TARGET_OBJECTS:image_converter_core>)
target_link_libraries(image_converter PRIVATE skia_lib)
target_link_options(image_converter PRIVATE ${COMMON_LINK_OPTIONS})
set_target_properties(image_converter PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/packages/core/dist"
    OUTPUT_NAME "wasm-image-optimization"
    SUFFIX ".js"
)

add_executable(image_converter_single $<TARGET_OBJECTS:image_converter_core>)
target_link_libraries(image_converter_single PRIVATE skia_lib)
target_link_options(image_converter_single PRIVATE ${COMMON_LINK_OPTIONS} "-sSINGLE_FILE=1" "-sENVIRONMENT=web,worker,node")
set_target_properties(image_converter_single PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/packages/core/dist"
    OUTPUT_NAME "wasm-image-optimization-single"
    SUFFIX ".js"
)
