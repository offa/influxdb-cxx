#!/bin/bash
# test including influxdb-cxx

set -ex
PID=$$
BASEPATH=/tmp/influx-test-${PID}
BUILD_TYPE="Release"
export PATH=$HOME/.local/bin:$PATH

echo "perform deployment test in ${BASEPATH}"
mkdir -p ${BASEPATH}/influxdb-cxx
cp -r ./* ${BASEPATH}/influxdb-cxx
cp -r script/include_library/* ${BASEPATH}/

cd /tmp/influx-test-${PID}/
mkdir build && cd build

conan install \
    -of . \
    -g CMakeToolchain \
    -g CMakeDeps \
    --build=missing \
    -s build_type=${BUILD_TYPE} \
    -s compiler.cppstd=17 \
    --requires=cpr/1.10.0

cmake -DCMAKE_TOOLCHAIN_FILE=./conan_toolchain.cmake -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" "$@" ..
cmake --build . -j

#cleanup
rm -r ${BASEPATH}
