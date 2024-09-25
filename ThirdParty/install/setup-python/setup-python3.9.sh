#!/bin/bash

VERSION=3.9.13

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

./configure --prefix=/usr/local/python3

make && make altinstall

rm -rf /usr/bin/python /usr/bin/python3 /usr/bin/pip /usr/bin/pip3

ln -s /usr/local/python3/bin/python3.9 /usr/bin/python
ln -s /usr/local/python3/bin/python3.9 /usr/bin/python3
ln -s /usr/local/python3/bin/pip3.9 /usr/bin/pip
ln -s /usr/local/python3/bin/pip3.9 /usr/bin/pip3