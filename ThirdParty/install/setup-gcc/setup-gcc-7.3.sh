#!/bin/bash

detectedubuntu=`apt --version`
if [ "$detectedubuntu" == "" ]; then
    onubuntu='FALSE'
else
    onubuntu='TRUE'
fi

if [ "$onubuntu" == "FALSE" ]; then
    yum -y install centos-release-scl
    yum -y install devtoolset-7-gcc devtoolset-7-gcc-c++ devtoolset-7-binutils
    scl enable devtoolset-7 bash
    echo "source /opt/rh/devtoolset-7/enable" >> /etc/profile
else
    add-apt-repository ppa:ubuntu-toolchain-r/test
    apt-get update
    apt-get install gcc-7
    apt-get install g++-7
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 100
    update-alternatives --config gcc
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-7 100
    update-alternatives --config g++
    gcc -v
fi
