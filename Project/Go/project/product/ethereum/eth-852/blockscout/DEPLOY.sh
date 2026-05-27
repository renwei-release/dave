#!/bin/bash
#/*
# * Copyright (c) 2024 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

HOMEPATH=$(cd `dirname $0`; pwd)

action=$1
if [ -z "$action" ]; then
  action="up"
fi

if [ ! -f docker-compose ]; then
    curl -SL https://github.com/docker/compose/releases/download/v2.20.2/docker-compose-linux-x86_64 -o docker-compose
fi
chmod a+x docker-compose

version=6.5.0

cd blockscout-${version}-beta/docker-compose

${HOMEPATH}/docker-compose up -d