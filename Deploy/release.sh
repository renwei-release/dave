#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

PROJECTNAME=$1
PROJECTMAPPING=$2
RUNNINGFILE=/project/dave-running.sh
File=$(basename $0)

if [ ! "$PROJECTMAPPING" != "" ]; then
   ACTIONLINE=`docker exec -t ${PROJECTNAME} cat -n ${RUNNINGFILE} | grep 'action=debug' | awk '{print $1}'`
   if [ -n "$ACTIONLINE" ]; then
      echo -e "${File} modify ${PROJECTNAME} to \033[35mrelease\033[0m"
      ACTIONLINEARRAY=(${ACTIONLINE})
      docker exec -t ${PROJECTNAME} sed -i "${ACTIONLINEARRAY[0]}c action=release" ${RUNNINGFILE}
   fi
else
   echo -e "There is a folder map with the host, run with the default mode!"
fi