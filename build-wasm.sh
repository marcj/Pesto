#!/bin/bash

EMSDK="libs/emsdk/"
#eg 1.37.27
EM_VERSION="1.38.6"
source ${EMSDK}/emsdk_env.sh

${EMSDK}/emscripten/${EM_VERSION}/emcc -O3 -std=c++1z \
-I libs/skia/include/core \
-I libs/skia/include/config \
-I libs/skia/include/gpu \
-I libs/skia/include/utils \
-I libs/skia/third_party/externals/sdl/include/ \
-I libs/skia/src/gpu \
-I libs/skia/src/core \
-I libs/skia/src/image/ \
-I libs/skia/tools/ \
-I libs/skia/include/views/ \
-I libs/skia/include/effects/ \
-I libs/skia/include/private/ \
-I libs/skia/tools/timer/ \
-I libs/algorithm/include/ \
-g all
src/gui/demo/example_simple.cpp \
-o build/pesto_demo_example_simple.bc

echo "Build app.html ...";

${EMSDK}/emscripten/${EM_VERSION}/emcc -O3 -std=c++1z \
build/pesto_demo_example_simple.bc \
build/emscripten/libpesto.a \
build/emscripten/xml/libtinyxml2.a \
libs/skia/out/Build-wasm-Release/Release/libskia.a \
-s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]'  \
-s ALLOW_MEMORY_GROWTH=1 -s USE_FREETYPE=1 -s USE_LIBPNG=1 -s WASM=1 -s FORCE_FILESYSTEM=0 \
-s DISABLE_EXCEPTION_CATCHING=2 \
-s NO_EXIT_RUNTIME=1 \
-s USE_WEBGL2=1 \
-g all
-o build/app.html