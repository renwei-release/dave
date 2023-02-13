#!/bin/bash
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

VERSION=0.6.3

PROJECT=$1
HOMEPATH=$2
PROJECTDIR=`pwd`/../../../../../Deploy/deploy/${PROJECT,,}/file_system/project
PRIJECTBIN=${PROJECT^^}-BIN
POLYGONBIN=`pwd`/polygon-edge-${VERSION}/polygon-edge
INIT=`pwd`/INIT
File=$(basename $0)

if [ ! -d ./polygon-edge-${VERSION} ]; then
   if [ ! -f v${VERSION}.tar.gz ]; then
      wget https://github.com/0xPolygon/polygon-edge/archive/refs/tags/v${VERSION}.tar.gz
   fi

   tar -zxvf v${VERSION}.tar.gz
   rm -rf v${VERSION}.tar.gz
fi

cd polygon-edge-${VERSION}

make build

cd ../

if [ ! -d ${PROJECTDIR} ]; then
   mkdir -p ${PROJECTDIR}
fi

echo ${File} copy ${POLYGONBIN} to ${PROJECTDIR}
chmod a+x ${POLYGONBIN}
cp ${POLYGONBIN} ${PROJECTDIR}

echo ${File} copy ${INIT} to ${PROJECTDIR}
chmod a+x ${INIT}
cp ${INIT} ${PROJECTDIR}

echo ${File} copy ${PRIJECTBIN} to ${PROJECTDIR}
chmod a+x ${PRIJECTBIN}
cp ${PRIJECTBIN} ${PROJECTDIR}