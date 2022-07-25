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
DEPLOYMODEL=$2

SHHOMEPATH=$(cd `dirname $0`; pwd)

if [[ "$DEPLOYMODEL" == "" ]] || [[ "$DEPLOYMODEL" == "all" ]]; then
   cp log_Dockerfile Dockerfile
   IMAGE="log_docker_image"
   TAG="latest"
   EXTEND="-v /dave/log:/dave/log"
   cd ../../
   chmod a+x *.sh
   ./deploy.sh -p ${PROJECT} -n ${PROJECT}-log -i ${IMAGE} -t ${TAG} -e "$EXTEND" -h ${HOMEPATH}
   cd ${SHHOMEPATH}
   rm -rf Dockerfile
fi

if [[ "$DEPLOYMODEL" == "es" ]] || [[ "$DEPLOYMODEL" == "all" ]]; then
   exit_es_contains=`docker ps -a | grep -w "${PROJECT}-es"`
   if [ "$exit_es_contains" == "" ]; then
      docker run -d --name ${PROJECT}-es --restart always --network host -e "discovery.type=single-node" elasticsearch:8.3.2
      # 似乎elasticsearch有一个BUG，
      # 不在初始配置下工作一段时间，生成certs下面的相关文件后，
      # 才能转为elasticsearch.yml的xpack.security.enabled: false
      # 模式（无需用户认证，且为HTTP方式访问）工作。
      sleep 3m
   fi
   docker cp ./file_system/usr/share/elasticsearch/config/elasticsearch.yml log-es:/usr/share/elasticsearch/config
   docker restart ${PROJECT}-es
fi

if [[ "$DEPLOYMODEL" == "stash" ]] || [[ "$DEPLOYMODEL" == "all" ]]; then
   cp logstash_Dockerfile Dockerfile
   IMAGE="logstash_docker_image"
   TAG="latest"
   EXTEND="-v /dave/log:/dave/log"
   cd ../../
   chmod a+x *.sh
   ./deploy.sh -p ${PROJECT} -n ${PROJECT}-logstash -c "FALSE" -i ${IMAGE} -t ${TAG} -e "$EXTEND" -h ${HOMEPATH}
   cd ${SHHOMEPATH}
   rm -rf Dockerfile
fi


if [[ "$DEPLOYMODEL" == "kibana" ]] || [[ "$DEPLOYMODEL" == "all" ]]; then
   cp kibana_Dockerfile Dockerfile
   IMAGE="kibana_docker_image"
   TAG="latest"
   EXTEND="-v /dave/log:/dave/log -e ELASTICSEARCH_HOSTS=http://127.0.0.1:9200"
   cd ../../
   chmod a+x *.sh
   ./deploy.sh -p ${PROJECT} -n ${PROJECT}-kibana -c "FALSE" -i ${IMAGE} -t ${TAG} -e "$EXTEND" -h ${HOMEPATH}
   cd ${SHHOMEPATH}
   rm -rf Dockerfile
   echo -e "Now \033[35mkibana\033[0m is ready!"
   echo -e "Please browse the web: \033[35mhttp://[your IP address]:5601/app/management/data/index_management/indices\033[0m"
fi

if [[ "$DEPLOYMODEL" == "jaeger" ]] || [[ "$DEPLOYMODEL" == "all" ]]; then
   cp jaeger_Dockerfile Dockerfile
   IMAGE="jaeger_docker_image"
   TAG="latest"
   EXTEND="-v /dave/log:/dave/log"
   cd ../../
   chmod a+x *.sh
   ./deploy.sh -p ${PROJECT} -n ${PROJECT}-jaeger -c "FALSE" -i ${IMAGE} -t ${TAG} -e "$EXTEND" -h ${HOMEPATH}
   cd ${SHHOMEPATH}
   rm -rf Dockerfile
   echo -e "Now \033[35mjaeger\033[0m is ready!"
fi