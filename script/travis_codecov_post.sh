#/bin/bash

set -ex

cd ${TRAVIS_BUILD_DIR}/build
lcov --directory . --capture --output-file coverage.info
lcov --remove coverage.info '/opt/*' '/usr/*' --output-file coverage.info
lcov --list coverage.info
bash <(curl -s https://codecov.io/bash) || echo "No coverage reports"
