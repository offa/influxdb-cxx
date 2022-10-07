#!/bin/bash

set -ex

mkdir -p build && cd build

cmake -DCMAKE_TOOLCHAIN_FILE=./conan_paths.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    "$@" ..
cmake --build . -j
