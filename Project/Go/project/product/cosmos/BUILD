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
CHAINNAME="planet"
MODELNAME="blog"
File=$(basename $0)
ARCH=`arch`
IGNITEENABLE="False"

if [ "$ARCH" == "x86_64" ]; then
   GOARCH=amd64
elif [ "$ARCH" == "aarch64" ]; then
   GOARCH=arm64
else
   echo Please define the GOARCH!
fi

go env -w GOARCH=${GOARCH}

if [ ${IGNITEENABLE} == "True" ]; then
   chmod a+x ignite-BUILD
   ./ignite-BUILD ${PROJECTDIR}
   chmod a+x model-BUILD
   ./model-BUILD ${CHAINNAME} ${MODELNAME} ${PROJECTDIR}
fi
chmod a+x gaia-BUILD
./gaia-BUILD ${PROJECTDIR}
chmod a+x relayer-BUILD
./relayer-BUILD ${PROJECTDIR}

#echo ${File} copy ${PRIJECTBIN} to ${PROJECTDIR}
#chmod a+x ${PRIJECTBIN}
#cp ${PRIJECTBIN} ${PROJECTDIR}