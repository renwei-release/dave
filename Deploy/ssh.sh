#!/bin/bash
#/*
# * Copyright (c) 2025 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

PROJECTNAME=$1
SSHPORT=$2
SSHCFGFILE=/etc/ssh/sshd_config
File=$(basename $0)

if [ "$SSHPORT" != "0" ]; then
   echo ${File} find ssh, replece ${PROJECTNAME} ssh port to ${SSHPORT}
   SSHPORTLINE=`docker exec -t ${PROJECTNAME} cat -n ${SSHCFGFILE} | grep 'Port ' | head -n 1 | awk '{print $1}'`
   docker exec -t ${PROJECTNAME} sed -i "${SSHPORTLINE}c Port ${SSHPORT}" ${SSHCFGFILE}
fi
