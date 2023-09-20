#!/bin/bash
#/*
# * Copyright (c) 2023 Renwei
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

if [ "$DEPLOYMODEL" == "" ]; then
   DEPLOYMODEL="hub"
fi

if [[ "$DEPLOYMODEL" == "wallet" ]]; then
   cp v-wallet-Dockerfile Dockerfile
   IMAGE="vwallet_docker_image"
   TAG="latest"
   cd ../../
   chmod a+x *.sh
   ./deploy.sh -p ${PROJECT} -n ${PROJECT}-vwallet -c "FALSE" -i ${IMAGE} -t ${TAG} -e "$EXTEND" -h ${HOMEPATH}
   cd ${SHHOMEPATH}
   rm -rf Dockerfile
fi

if [[ "$DEPLOYMODEL" == "system" ]]; then
   cp v-system-Dockerfile Dockerfile
   IMAGE="vsystem_docker_image"
   TAG="latest"
   cd ../../
   chmod a+x *.sh
   ./deploy.sh -p ${PROJECT} -n ${PROJECT}-vsystem -c "FALSE" -i ${IMAGE} -t ${TAG} -e "$EXTEND" -h ${HOMEPATH}
   cd ${SHHOMEPATH}
   rm -rf Dockerfile
fi

if [[ "$DEPLOYMODEL" == "hub" ]]; then
   cd ../mongodb
   ./deploy.sh
   cd ${SHHOMEPATH}

   cp MultiHUB-Dockerfile Dockerfile
   IMAGE="hub_docker_image"
   TAG="latest"
   cd ../../
   chmod a+x *.sh
   ./deploy.sh -p ${PROJECT} -n ${PROJECT}-hub -c "FALSE" -i ${IMAGE} -t ${TAG} -e "$EXTEND" -h ${HOMEPATH}
   cd ${SHHOMEPATH}
   rm -rf Dockerfile
fi

if [[ "$DEPLOYMODEL" == "indexer" ]] || [[ "$DEPLOYMODEL" == "kafka" ]]; then
   if [ -f pure-indexer-deploy.sh ]; then
      chmod a+x pure-indexer-deploy.sh
      ./pure-indexer-deploy.sh $DEPLOYMODEL
   fi
fi