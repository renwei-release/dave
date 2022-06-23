#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

PROJECT=${PWD##*/}
if [ "$1" == "" ]; then
   HOMEPATH=$(cd `dirname $0`; pwd)
else
   HOMEPATH=$1
fi
IMAGE="ipfs_docker_image"
TAG="latest"
EXTEND=""

cd ../../
chmod a+x *.sh
./deploy.sh -p ${PROJECT} -i ${IMAGE} -t ${TAG} -e "$EXTEND" -h ${HOMEPATH}