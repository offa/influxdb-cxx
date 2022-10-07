#!/bin/bash

set -ex

apt-get update
apt-get install -y python3-pip libcurl4-openssl-dev
pip3 install -U conan

mkdir -p build && cd build

conan profile new default --detect
conan profile update settings.compiler.cppstd=17 default

if [[ "${CXX}" == clang* ]]
then
    STDLIB_ENV="CXXFLAGS=\"-stdlib=libc++\""
    export ${STDLIB_ENV}
    echo "${STDLIB_ENV}" >> ${GITHUB_ENV}
    conan profile update settings.compiler.libcxx=libc++ default
else
    conan profile update settings.compiler.libcxx=libstdc++11 default
fi


conan install \
    -o influxdb-cxx:system=True \
    -o influxdb-cxx:tests=True \
    --build=missing \
    ..
