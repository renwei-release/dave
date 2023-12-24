#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

VERSION=1.13.5

PROJECT=$1
HOMEPATH=$2
PROJECTDIR=`pwd`/../../../../../Deploy/deploy/${PROJECT,,}/file_system/project
GETHBIN=`pwd`/go-ethereum-${VERSION}/build/bin/geth
PRIJECTBIN=${PROJECT^^}-BIN
PRYSM=`pwd`/prysm
File=$(basename $0)

if [ ! -d ./go-ethereum-${VERSION} ]; then
   if [ ! -f v${VERSION}.tar.gz ]; then
      wget https://github.com/ethereum/go-ethereum/archive/refs/tags/v${VERSION}.tar.gz
   fi

   tar -zxvf v${VERSION}.tar.gz
   rm -rf v${VERSION}.tar.gz
fi

cd go-ethereum-${VERSION}

make geth

cd ../

if [ ! -d ${PROJECTDIR} ]; then
   mkdir -p ${PROJECTDIR}
fi

echo ${File} copy ${GETHBIN} to ${PROJECTDIR}
chmod a+x ${GETHBIN}
cp ${GETHBIN} ${PROJECTDIR}

echo ${File} copy ${PRYSM} to ${PROJECTDIR}
chmod a+x ${PRYSM}
cp -r ${PRYSM} ${PROJECTDIR}

echo ${File} copy ${PRIJECTBIN} to ${PROJECTDIR}
chmod a+x ${PRIJECTBIN}
cp ${PRIJECTBIN} ${PROJECTDIR}