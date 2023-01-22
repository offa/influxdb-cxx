#!/bin/bash
# test including influxdb-cxx

set -ex
PID=$$
BASEPATH=/tmp/influx-test-${PID}

echo "perform deployment test in ${BASEPATH}"
mkdir -p ${BASEPATH}/influxdb-cxx
cp -r ./* ${BASEPATH}/influxdb-cxx
cp -r script/include_library/* ${BASEPATH}/

cd /tmp/influx-test-${PID}/
mkdir build && cd build

conan install -g cmake_paths -g cmake_find_package cpr/1.9.3@

cmake -DCMAKE_TOOLCHAIN_FILE=./conan_paths.cmake "$@" ..
cmake --build . -j

#cleanup
rm -r ${BASEPATH}
