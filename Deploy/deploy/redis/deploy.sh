#!/bin/bash
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

PROJECT=dave-${PWD##*/}
HOMEPATH=$(cd `dirname $0`; pwd)
IMAGE="redis_docker_image"
TAG="latest"

exit_redis_contains=`docker ps -a | grep -w "${PROJECT}"`
if [ "$exit_redis_contains" == "" ]; then
   docker run --name ${PROJECT} -p 6379:6379 --restart always -d redis:7.2.3

   docker restart ${PROJECT}
fi