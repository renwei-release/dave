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

publicloaderror=true

if [ -f public-load.sh ]; then
   if [ -f ./deploy/${PROJECT}/Dockerfile ]; then
      ./public-load.sh $PROJECT ${IMAGE} ${TAG}
      publicloaderror=false
   fi
fi

if [ $publicloaderror = true ]; then
   echo ${PROJECT} "No public-load.sh or Dockerfile file found"
fi