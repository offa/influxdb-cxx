#!/bin/bash

set -ex

apt-get update

# #129: Workaround for broken python3-pip package
curl -sSL https://bootstrap.pypa.io/get-pip.py -o get-pip.py
python3 get-pip.py

apt-get install -y libcurl4-openssl-dev
pip3 install -U conan

conan profile new default --detect

if [[ ${CC} == gcc* ]]; then
    conan profile update settings.compiler.libcxx=libstdc++11 default
fi

cp script/settings.yml ~/.conan/

conan install -o influxdb-cxx:system=True -o influxdb-cxx:tests=True .
