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

if [[ "$DEPLOYMODEL" == "server" ]] || [[ "$DEPLOYMODEL" == "all" ]]; then
   exit_server_contains=`docker ps -a | grep -w "${PROJECT}-server"`
   if [ "$exit_server_contains" == "" ]; then
      SWARM_PORT=4002
	  API_PORT=5001
	  GATEWAY_PORT=8088
      # Swarm listening on ${SWARM_PORT}/tcp/udp
      # API server listening on ${API_PORT}
	  # WebUI: http://0.0.0.0:${API_PORT}/webui
      docker run -d --name ${PROJECT}-server --ulimit nofile=4096 -p ${SWARM_PORT}:4001/tcp -p ${SWARM_PORT}:4001/udp -p ${API_PORT}:5001 -p ${GATEWAY_PORT}:8080 --restart always ipfs/go-ipfs:master-2022-10-19-8c72ea9
   fi
fi

if [[ "$DEPLOYMODEL" == "" ]] || [[ "$DEPLOYMODEL" == "client" ]] || [[ "$DEPLOYMODEL" == "all" ]]; then
   cp client_Dockerfile Dockerfile
   IMAGE="ipfs_client_docker_image"
   TAG="latest"
   EXTEND=""
   cd ../../
   chmod a+x *.sh
   ./deploy.sh -p ${PROJECT} -n ${PROJECT}-client -i ${IMAGE} -t ${TAG} -e "$EXTEND" -h ${HOMEPATH}
   cd ${SHHOMEPATH}
   rm -rf Dockerfile
fi