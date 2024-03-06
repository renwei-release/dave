#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

PROJECTNAME=$1
PROJECT=$2
IMAGE=$3
TAG=$4
REMOVEDIR=dave/${PROJECT,,}/config
File=$(basename $0)

if [ "$TAG" == "" ]; then
   TAG="latest"
fi

#
# 删除僵尸容器
#
zombie_container=`docker ps -a | awk -v name=${PROJECTNAME} '$NF==name' | grep "Exited" | head -n 1`
if [ "$zombie_container" ]; then
   echo "*** Found zombie container. remove it!"
   docker stop ${PROJECTNAME}
   docker rm ${PROJECTNAME}
fi

#
# 检测是否容器对应的镜像版本有变化，如果有变化，则删除容器。
#
container_image_name=`docker ps -a | awk -v name=${PROJECTNAME} '$NF==name' | awk '{print $2}' | head -n 1`
if [ "$container_image_name" != "" ]; then
   current_image_name=${IMAGE}:${TAG}
   if [ $container_image_name != $current_image_name ]; then
      echo "${File} *** Found that the container version($container_image_name $current_image_name) is inconsistent!"
      echo ${File} copy ${REMOVEDIR} directory from ${PROJECTNAME}
      docker cp ${PROJECTNAME}:/${REMOVEDIR} ./
      docker stop ${PROJECTNAME}
      docker rm ${PROJECTNAME}
   fi
fi