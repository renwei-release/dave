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

if [ "$TAG" == "" ]; then
   exit_dave_image=`docker image ls | grep ${IMAGE}`
else
   exit_dave_image=`docker image ls | grep ${IMAGE} | grep ${TAG}`
fi

if [ "$exit_dave_image" == "" ]; then
   if [ -f ./deploy/${PROJECT}/Dockerfile ]; then
      echo public-load.sh build ${IMAGE} on ${IMAGE}:${TAG}
      cp dave-running.sh ./deploy/${PROJECT}/dave-running.sh
      docker build --tag ${IMAGE}:${TAG} ./deploy/${PROJECT}
      rm ./deploy/${PROJECT}/dave-running.sh
   else
      echo public-load.sh "can't find ./deploy/${PROJECT}/Dockerfile"
      exit 1
   fi
fi