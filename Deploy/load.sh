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

publicloadflag=true

if [ -f public-load.sh ]; then
   if [ -f ./deploy/${PROJECT}/Dockerfile ]; then
      publicloadflag=false
      ./public-load.sh $PROJECT ${IMAGE} ${TAG}
   fi
fi

if [ "$publicloadflag" == "true" ]; then
   if [ -f private-load.sh ]; then
      ./private-load.sh ${IMAGE} ${TAG}
   fi
fi