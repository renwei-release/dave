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

exit_contains=`docker ps -a | grep -w "${PROJECT}"`
if [ "$exit_contains" == "" ]; then
   SWARM_PORT=4002
   API_PORT=5001
   GATEWAY_PORT=8088
   FROM=ipfs/go-ipfs:master-2022-10-19-8c72ea9
   # Swarm listening on ${SWARM_PORT}/tcp/udp
   # API server listening on ${API_PORT}
   # WebUI: http://127.0.0.1:${API_PORT}/webui
   docker run -d --name ${PROJECT} --ulimit nofile=4096 -p ${SWARM_PORT}:4001/tcp -p ${SWARM_PORT}:4001/udp -p 127.0.0.1:${API_PORT}:5001 -p 127.0.0.1:${GATEWAY_PORT}:8080 --restart always ${FROM}
   sleep 10s
   docker exec -it ${PROJECT} sed -i 's/    "API": "\/ip4\/0.0.0.0\/tcp\/5001",/    "API": "\/ip4\/127.0.0.1\/tcp\/5001",/g' /data/ipfs/config
   docker exec -t ${PROJECT} sed -i 's/    "Gateway": "\/ip4\/0.0.0.0\/tcp\/8080",/    "Gateway": "\/ip4\/127.0.0.1\/tcp\/8080",/g' /data/ipfs/config
fi
docker restart ${PROJECT}