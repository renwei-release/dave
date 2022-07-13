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
IMAGE="test_docker_image"
TAG="latest"
EXTEND=""

cd ../../
chmod a+x *.sh
./deploy.sh -p ${PROJECT} -i ${IMAGE} -t ${TAG} -e "$EXTEND" -h ${HOMEPATH}

echo -e "Now \033[35mjenkins\033[0m is ready!"
echo -e "Please browse the web: \033[35mhttp://{your IP address}:8080/login?from=%2F\033[0m"
echo -e "The website User: \033[35mtest\033[0m Password: \033[35m000000\033[0m"