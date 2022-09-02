#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

PROJECT=${PWD##*/}
DEPLOYMODEL=$1
if [ "$2" == "" ]; then
   HOMEPATH=$(cd `dirname $0`; pwd)
else
   HOMEPATH=$2
fi
SHHOMEPATH=$(cd `dirname $0`; pwd)

if [[ "$DEPLOYMODEL" == "etcd" ]] || [[ "$DEPLOYMODEL" == "all" ]]; then
   cp etcd_Dockerfile Dockerfile
   IMAGE="etcd_docker_image"
   TAG="latest"
   EXTEND="-e ETCD_NAME=sync-etcd -e ALLOW_NONE_AUTHENTICATION=yes"
   cd ../../
   chmod a+x *.sh
   ./deploy.sh -p ${PROJECT} -n ${PROJECT}-etcd -c "FALSE" -i ${IMAGE} -t ${TAG} -e "$EXTEND" -h ${HOMEPATH}
   cd ${SHHOMEPATH}
   rm -rf Dockerfile
fi

if [[ "$DEPLOYMODEL" == "etcdkeeper" ]] || [[ "$DEPLOYMODEL" == "all" ]]; then
   cp etcdkeeper_Dockerfile Dockerfile
   IMAGE="etcdkeeper_docker_image"
   TAG="latest"
   EXTEND=""
   cd ../../
   chmod a+x *.sh
   ./deploy.sh -p ${PROJECT} -n ${PROJECT}-etcdkeeper -c "FALSE" -i ${IMAGE} -t ${TAG} -e "$EXTEND" -h ${HOMEPATH}
   cd ${SHHOMEPATH}
   rm -rf Dockerfile
   echo -e "Now \033[35metcdkeeper\033[0m is ready!"
   echo -e "Please browse the web: \033[35mhttp://[your IP address]:8080/etcdkeeper\033[0m"
fi

if [[ "$DEPLOYMODEL" == "" ]] || [[ "$DEPLOYMODEL" == "sync" ]] || [[ "$DEPLOYMODEL" == "all" ]]; then
   cp sync_Dockerfile Dockerfile
   IMAGE="sync_docker_image"
   TAG="latest"
   EXTEND="-v /dave/sync:/dave/sync"
   cd ../../
   chmod a+x *.sh
   ./deploy.sh -p ${PROJECT} -n ${PROJECT}-sync -i ${IMAGE} -t ${TAG} -e "$EXTEND" -h ${HOMEPATH}
   cd ${SHHOMEPATH}
   rm -rf Dockerfile
fi