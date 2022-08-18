#!/bin/bash

set -ex

apt-get update
apt-get install -y python3-pip libcurl4-openssl-dev
pip3 install -U conan

conan profile new default --detect

if [[ ${CC} == gcc* ]]; then
    conan profile update settings.compiler.libcxx=libstdc++11 default
fi

cp script/settings.yml ~/.conan/

conan install -o influxdb-cxx:system=True -o influxdb-cxx:tests=True .
