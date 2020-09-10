#/bin/bash

if [[ "${TRAVIS_OS_NAME}" == "linux" ]]; then
    wget https://dl.influxdata.com/influxdb/releases/influxdb_1.8.2_amd64.deb
    sudo dpkg -i influxdb_1.8.2_amd64.deb
    sudo systemctl unmask influxdb.service
    sudo systemctl start influxdb
fi

if [[ "${TRAVIS_OS_NAME}" == "osx" ]]; then
    HOMEBREW_NO_AUTO_UPDATE=1 brew install influxdb
    ln -sfv /usr/local/opt/influxdb/*.plist ~/Library/LaunchAgents
    launchctl load ~/Library/LaunchAgents/homebrew.mxcl.influxdb.plist
fi

sleep 10
influx -execute 'CREATE DATABASE test'
