#!/bin/bash

set -ex


BUILD_OS="$1"

case "${BUILD_OS}" in
    -linux)
        apt-get install -y influxdb
        influxd &
        sleep 10
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
