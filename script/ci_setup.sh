#!/bin/bash

set -ex

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
    -o "&:system=True" \
    -o "&:tests=True" \
    -s compiler.cppstd=20 \
    --build=missing \
    ..
