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
   yum install -y libffi-devel
else
   apt-get install -y libffi-dev
fi

./configure --prefix=/usr/local/python3

make && make altinstall

rm -rf /usr/bin/python /usr/bin/python3 /usr/bin/pip /usr/bin/pip3

ln -s /usr/local/python3/bin/python3.9 /usr/bin/python
ln -s /usr/local/python3/bin/python3.9 /usr/bin/python3
ln -s /usr/local/python3/bin/pip3.9 /usr/bin/pip
ln -s /usr/local/python3/bin/pip3.9 /usr/bin/pip3

if [ "$onubuntu" == "FALSE" ]; then
   sed 's/python/python2.7/g' /usr/bin/yum >> yum.temp
   sed 's/python/python2.7/g' /usr/libexec/urlgrabber-ext-down >> urlgrabber-ext-down.temp
   rm /usr/bin/yum
   rm /usr/libexec/urlgrabber-ext-down
   mv yum.temp /usr/bin/yum
   mv urlgrabber-ext-down.temp /usr/libexec/urlgrabber-ext-down
   chmod a+x /usr/bin/yum /usr/libexec/urlgrabber-ext-down
fi