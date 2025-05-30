#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

PROJECT=$1
IMAGE=$2
TAG=$3
File=$(basename $0)
RUNNINGPATH=./deploy/${PROJECT}/file_system/project
RUNNINGFILE=dave-running.sh
DOCKERFILE=./deploy/${PROJECT}/Dockerfile

if [ "$TAG" == "" ]; then
   exit_dave_image=`docker image ls | grep -w ${IMAGE}`
else
   exit_dave_image=`docker image ls | grep -w ${IMAGE} | grep ${TAG}`
fi

if [ -f ${RUNNINGPATH}/${RUNNINGFILE} ]; then
   chmod a+x ${RUNNINGPATH}/${RUNNINGFILE}
fi

BUILDARG=$(./extend.sh 'build' '')

if [ "$exit_dave_image" == "" ]; then
   if [ -f ${DOCKERFILE} ]; then
      echo ${File} build ${IMAGE} on ${IMAGE}:${TAG} BUILDARG=${BUILDARG}
	   mkdir -p ${RUNNINGPATH}
      chmod a+x ${RUNNINGFILE}
      cp ${RUNNINGFILE} ${RUNNINGPATH}
      docker build ${BUILDARG} --tag ${IMAGE}:${TAG} ./deploy/${PROJECT}
   else
      echo ${File} "can't find ${DOCKERFILE}"
      exit 1
   fi
fi