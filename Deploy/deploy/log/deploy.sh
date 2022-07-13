#!/bin/bash
#/*
# * Copyright (c) 2022 Chenxiaomin
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

SHHOMEPATH=$(cd `dirname $0`; pwd)

cp log_Dockerfile Dockerfile
IMAGE="log_docker_image"
TAG="latest"
EXTEND="-v /dave/log:/dave/log"
cd ../../
chmod a+x *.sh
./deploy.sh -p ${PROJECT} -n ${PROJECT}-log -i ${IMAGE} -t ${TAG} -e "$EXTEND" -h ${HOMEPATH}
cd ${SHHOMEPATH}
rm -rf Dockerfile

exit_es_contains=`docker ps -a | grep -w "${PROJECT}-es"`
if [ "$exit_es_contains" == "" ]; then
   docker run -d --name ${PROJECT}-es --network host -e "discovery.type=single-node" elasticsearch:8.3.2
   # 似乎elasticsearch有一个BUG，
   # 不在初始配置下工作一段时间，生成certs下面的相关文件，
   # 才能转为elasticsearch.yml的xpack.security.enabled: false
   # 模式（无需用户认证，且为HTTP方式访问）工作。
   sleep 3m
fi
docker cp ./file_system/usr/share/elasticsearch/config/elasticsearch.yml log-es:/usr/share/elasticsearch/config
docker restart ${PROJECT}-es

cp logstash_Dockerfile Dockerfile
IMAGE="logstash_docker_image"
TAG="latest"
EXTEND="-v /dave/log:/dave/log"
cd ../../
chmod a+x *.sh
./deploy.sh -p ${PROJECT} -n ${PROJECT}-logstash -c "FALSE" -i ${IMAGE} -t ${TAG} -e "$EXTEND" -h ${HOMEPATH}
cd ${SHHOMEPATH}
rm -rf Dockerfile