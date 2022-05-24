#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

PROJECTNAME=$1
PROJECT=$2
RUNNINGFILE=/project/dave-running.sh
PRJBINFILE=./deploy/${PROJECT}/project/${PROJECT^^}-BIN
DEBUGBIN=./deploy/debug/project/DEBUG-BIN

ACTIONLINE=`docker exec -t ${PROJECTNAME} cat -n ${RUNNINGFILE} | grep 'action=release' | awk '{print $1}'`
if [ -n "$ACTIONLINE" ]; then
   echo -e "debug.sh modify ${PROJECTNAME} to \033[35mdebug\033[0m"
   ACTIONLINEARRAY=(${ACTIONLINE})
   docker exec -t ${PROJECTNAME} sed -i "${ACTIONLINEARRAY[0]}c action=debug" ${RUNNINGFILE}
fi

if [ -f ${PRJBINFILE} ]; then
   echo debug.sh copy ${PRJBINFILE} to ${PROJECTNAME}
   chmod a+x ${PRJBINFILE}
   docker cp ${PRJBINFILE} ${PROJECTNAME}:/project
fi

if [ -f ${DEBUGBIN} ]; then
   echo debug.sh copy ${DEBUGBIN} to ${PROJECTNAME}
   chmod a+x ${DEBUGBIN}
   docker cp ${DEBUGBIN} ${PROJECTNAME}:/project
fi

echo debug.sh restart ${PROJECTNAME}
docker restart ${PROJECTNAME}

echo vvvvvvvvvvvvvvvvv login ${PROJECTNAME} vvvvvvvvvvvvvvvvv
docker exec -it ${PROJECTNAME} bash