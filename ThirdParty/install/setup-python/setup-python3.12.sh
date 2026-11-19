#!/bin/bash

homepath=$(cd `dirname $0`; pwd)

VERSION=3.12.9

detectedubuntu=`apt --version`
if [ "$detectedubuntu" == "" ]; then
    onubuntu='FALSE'
else
    onubuntu='TRUE'
fi

if [ ! -f Python-${VERSION}.tgz ]; then
   wget https://www.python.org/ftp/python/${VERSION}/Python-${VERSION}.tgz
fi
if [ ! -d ./Python-${VERSION} ]; then
   tar zxvf Python-${VERSION}.tgz
fi

cd Python-${VERSION}

if [ "$onubuntu" == "FALSE" ]; then
   yum install -y libffi-devel libssl-dev
else
   apt-get install -y libffi-dev libssl-dev
fi

./configure --prefix=${homepath}/setup

make && make altinstall