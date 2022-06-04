#!/bin/sh

homepath=$(cd `dirname $0`; pwd)

if [ ! -f boost_1_73_0.tar.bz2 ]; then
   wget https://boostorg.jfrog.io/artifactory/main/release/1.73.0/source/boost_1_73_0.tar.bz2
fi
if [ ! -d boost_1_73_0 ]; then
   tar -jxvf boost_1_73_0.tar.bz2
fi

cd boost_1_73_0

./bootstrap.sh --with-libraries=all --with-toolset=gcc

./b2 toolset=gcc

./b2 install --prefix=/usr

cd $homepath

rm -rf boost_1_73_0