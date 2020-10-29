#/bin/bash

set -ex

mkdir build && cd build
cmake "$@" ..
cmake --build . -j
