#!/bin/bash

CONAN_BOOST_OPTION=True

if [[ -z ${NO_BOOST+x} ]]; then
    CONAN_BOOST_OPTION=False
fi

pip install -U --user conan
conan profile new default --detect
conan profile update settings.compiler.libcxx=libstdc++11 default
conan create -o influxdb-cxx:tests={CONAN_BOOST_OPTION} .
