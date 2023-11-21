#!/bin/bash
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

PROJECT=dave-${PWD##*/}
HOMEPATH=$(cd `dirname $0`; pwd)
IMAGE="elasticsearch_docker_image"
TAG="latest"

exit_es_contains=`docker ps -a | grep -w "${PROJECT}"`
if [ "$exit_es_contains" == "" ]; then
    docker run -d --name ${PROJECT} --restart always --network host -e "discovery.type=single-node" elasticsearch:8.3.2
    # 似乎elasticsearch有一个BUG，
    # 不在初始配置下工作一段时间，生成certs下面的相关文件后，
    # 才能转为elasticsearch.yml的xpack.security.enabled: false
    # 模式（无需用户认证，且为HTTP方式访问）工作。
    sleep 3m

    docker cp ./file_system/usr/share/elasticsearch/config/elasticsearch.yml ${PROJECT}:/usr/share/elasticsearch/config
    docker restart ${PROJECT}
fi