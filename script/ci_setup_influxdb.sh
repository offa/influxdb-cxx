#!/bin/bash

set -ex


BUILD_OS="$1"

case "${BUILD_OS}" in
    -linux)
        INFLUXDB_VERSION="1.8.3"
        wget https://dl.influxdata.com/influxdb/releases/influxdb_${INFLUXDB_VERSION}_amd64.deb
        sudo dpkg -i influxdb_${INFLUXDB_VERSION}_amd64.deb
        sudo systemctl unmask influxdb.service
        sudo systemctl start influxdb
        ;;
    -osx)
        HOMEBREW_NO_AUTO_UPDATE=1 brew install influxdb
        ln -sfv /usr/local/opt/influxdb/*.plist ~/Library/LaunchAgents
        launchctl load ~/Library/LaunchAgents/homebrew.mxcl.influxdb.plist
        sleep 10
        ;;
    *)
        echo "Invalid argument: '${BUILD_OS}'"
        exit 1
        ;;
esac
