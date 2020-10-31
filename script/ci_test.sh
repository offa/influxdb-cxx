#!/bin/bash

set -ex
cd build

# start influxdb
influxd &
INFLUX_PID=$!
sleep 10

# execute tests
ctest

# stop influx
kill $INFLUX_PID