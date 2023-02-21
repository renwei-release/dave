#!/bin/bash
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

PROJECT=${PWD##*/}
PROJECTDIR=`pwd`/../../../../../Deploy/deploy/${PROJECT,,}/file_system/project
PRIJECTBIN=COSMOS-BIN
File=$(basename $0)
ARCH=`arch`

if [ "$ARCH" == "x86_64" ]; then
   GOARCH=amd64
elif [ "$ARCH" == "aarch64" ]; then
   GOARCH=arm64
else
   echo Please define the GOARCH!
fi

go env -w GOARCH=${GOARCH}

chmod a+x gaia-BUILD
./gaia-BUILD ${PROJECTDIR}

echo ${File} copy ${PRIJECTBIN} to ${PROJECTDIR}
chmod a+x ${PRIJECTBIN}
cp ${PRIJECTBIN} ${PROJECTDIR}