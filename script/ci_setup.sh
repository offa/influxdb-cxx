#!/bin/bash

set -ex

export DEBIAN_FRONTEND=noninteractive
export PATH=$HOME/.local/bin:$PATH
apt-get update
apt-get install -y pipx
pipx install conan
conan profile detect

mkdir -p build && cd build

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
