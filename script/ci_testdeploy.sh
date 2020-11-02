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
cmake "$@" ..
cmake --build . -j

#cleanup
rm -r ${BASEPATH}