#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

PROJECTNAME=$1
File=$(basename $0)

exit_project_contains=`docker ps -a | grep -w "${PROJECTNAME}"`

# 如果容器存在且处于停机状态时就启动它，
# 如果容器不存在就不理会，
# 这是为了方便后续流程的工作，比如需要从原容器里面备份/dave目录等。
if [ ! "$exit_project_contains" == "" ]; then
   has_contains_but_exit=$(echo $exit_project_contains | grep "Exited")
   if [ ! "$has_contains_but_exit" == "" ]; then
      echo ${File} found a stopped container ${PROJECTNAME}, start it now ...
      docker start ${PROJECTNAME}
   fi
fi