#!/bin/sh

yum remove cmake

wget https://cmake.org/files/v3.13/cmake-3.13.0.tar.gz

tar xzvf cmake-3.13.0.tar.gz

cd cmake-3.13.0

./bootstrap

gmake

make install

ln -s /usr/local/bin/cmake /usr/bin/cmake
