#/bin/bash

set -ex

mkdir build && cd build
cmake ..
make -j
