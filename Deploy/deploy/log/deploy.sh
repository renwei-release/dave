#!/bin/bash
#/*
# * Copyright (c) 2022 Chenxiaomin
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

if [[ "$DEPLOYMODEL" == "" ]] || [[ "$DEPLOYMODEL" == "log" ]] || [[ "$DEPLOYMODEL" == "all" ]]; then
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
   cd ../elasticsearch
   ./deploy.sh
   cd ${SHHOMEPATH}
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
   echo -e "Please browse the web: \033[35mhttp://[your IP address]:16686/search\033[0m"
fi