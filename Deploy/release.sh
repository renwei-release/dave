#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

PROJECTNAME=$1
RUNNINGFILE=/project/dave-running.sh

ACTIONLINE=`docker exec -t ${PROJECTNAME} cat -n ${RUNNINGFILE} | grep 'action=debug' | awk '{print $1}'`
if [ -n "$ACTIONLINE" ]; then
   echo release.sh modify ${PROJECTNAME} to release
   ACTIONLINEARRAY=(${ACTIONLINE})
   docker exec -t ${PROJECTNAME} sed -i "${ACTIONLINEARRAY[0]}c action=release" ${RUNNINGFILE}
   docker restart ${PROJECTNAME}
fi