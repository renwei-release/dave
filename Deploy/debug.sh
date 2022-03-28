#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

PROJECTNAME=$1
RUNNINGFILE=/project/dave-running.sh
DEBUGBIN=./deploy/debug/DEBUG-BIN

ACTIONLINE=`docker exec -t ${PROJECTNAME} cat -n ${RUNNINGFILE} | grep 'action=release' | awk '{print $1}'`
if [ -n "$ACTIONLINE" ]; then
   echo debug.sh modify action to debug
   ACTIONLINEARRAY=(${ACTIONLINE})
   docker exec -t ${PROJECTNAME} sed -i "${ACTIONLINEARRAY[0]}c action=debug" ${RUNNINGFILE}
fi

if [ -f ${DEBUGBIN} ]; then
   echo debug.sh copy ${DEBUGBIN} to ${PROJECTNAME}
   chmod a+x ${DEBUGBIN}
   docker cp ${DEBUGBIN} ${PROJECTNAME}:/project/DEBUG-BIN
fi

echo debug.sh restart ${PROJECTNAME}
docker restart ${PROJECTNAME}

echo vvvvvvvvvvvvvvvvv login ${PROJECTNAME} vvvvvvvvvvvvvvvvv
docker exec -it ${PROJECTNAME} bash