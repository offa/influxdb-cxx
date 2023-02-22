#!/bin/bash

set -ex

mkdir -p build && cd build

cmake --preset conan-release "$@" ..
cmake --build . -j
cmake --build . --target unittest
