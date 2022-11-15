#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

VERSION=1.10.25

PROJECT=$1
HOMEPATH=$2
PROJECTDIR=`pwd`/../../../../../Deploy/deploy/${PROJECT,,}/file_system/project
PRIJECTFILE=${PROJECT^^}-BIN
GETHBIN=`pwd`/go-ethereum-${VERSION}/build/bin/geth
PRYSM=`pwd`/prysm

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

chmod a+x ${PRIJECTFILE}

echo BUILD copy ${GETHBIN} to ${PROJECTDIR}
cp ${GETHBIN} ${PROJECTDIR}

echo BUILD copy ${PRIJECTFILE} to ${PROJECTDIR}
cp ${PRIJECTFILE} ${PROJECTDIR}

echo BUILD copy ${PRYSM} to ${PROJECTDIR}
cp -r ${PRYSM} ${PROJECTDIR}