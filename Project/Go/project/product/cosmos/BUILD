#!/bin/bash
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

PROJECT=${PWD##*/}
FILESYSTEM=`pwd`/../../../../../Deploy/deploy/${PROJECT,,}/file_system
PROJECTDIR=${FILESYSTEM}/project
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

#
# Reference:
# https://hermes.informal.systems/tutorials/pre-requisites/gaiad-manager.html
#
chmod a+x gaia-BUILD
./gaia-BUILD ${PROJECTDIR}
chmod a+x sconfig-BUILD
./sconfig-BUILD ${PROJECTDIR}
chmod a+x stoml-BUILD
./stoml-BUILD ${PROJECTDIR}
chmod a+x gm-BUILD
./gm-BUILD ${PROJECTDIR}
chmod a+x hermes-BUILD
./hermes-BUILD ${PROJECTDIR}

echo ${File} copy ${PRIJECTBIN} to ${PROJECTDIR}
chmod a+x ${PRIJECTBIN}
cp ${PRIJECTBIN} ${PROJECTDIR}