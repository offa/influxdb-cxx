#!/bin/bash

set -ex

apt-get update
apt-get install -y python3-pip libssl-dev libcurl4-openssl-dev
pip3 install -U conan

mkdir -p build && cd build

conan profile detect

if [[ "${CXX}" == clang* ]]
then
    STDLIB_ENV="CXXFLAGS=\"-stdlib=libc++\""
    export ${STDLIB_ENV}
    echo "${STDLIB_ENV}" >> ${GITHUB_ENV}
    sed -i 's/^compiler.libcxx=.*$/compiler.libcxx=libc++/g' ~/.conan2/profiles/default
fi

conan install \
    -of . \
    -o "influxdb-cxx/*":system=True \
    -o "influxdb-cxx/*":tests=True \
    -s compiler.cppstd=17 \
    --build=missing \
    ..
