#!/bin/bash

set -ex

apt-get update
apt-get install -y python3-pip libcurl4-openssl-dev
pip3 install -U conan

if [[ ${CC} == gcc* ]]; then
    conan profile new default --detect
    conan profile update settings.compiler.libcxx=libstdc++11 default
fi

conan install -o influxdb-cxx:system=True -o influxdb-cxx:tests=True .
