#!/bin/sh

homepath=$(cd `dirname $0`; pwd)

VERSION=0.17.0

if [ ! -f v${VERSION}.tar.gz ]; then
  wget https://github.com/ipfs/kubo/archive/refs/tags/v${VERSION}.tar.gz
fi
if [ ! -d ./kubo-${VERSION} ]; then
  tar -zxvf v${VERSION}.tar.gz
fi





