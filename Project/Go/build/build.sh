#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

homepath=$(cd `dirname $0`; pwd)

PROJECT=$1
TAGS=$2
TIDY=$3

if [ "$PROJECT" == "" ]; then
   PROJECT=dave
fi
projectnameforbuild=projectname${PROJECT}

python3 ../../../Tools/refresh_version/refresh_version.py "../../../" ${PROJECT^^}


if [ -f tidy.sh ]; then
   ./tidy.sh ${PROJECT} ${TIDY}
fi

if [ -f $PROJECT ]; then
   rm -rf $PROJECT
fi

cd ../project

GOOS=linux GOARCH=amd64 go build -gcflags=all="-N -l" -tags "${TAGS} __DAVE_PRODUCT_${PROJECT^^}__" -o $projectnameforbuild dave_main.go

if [ -f $projectnameforbuild ]; then
   PROJECTDIR=../../../Deploy/deploy/${PROJECT,,}/file_system/project
   PRIJECTFILE=${PROJECTDIR}/${PROJECT^^}-BIN

   if [ ! -d ${PROJECTDIR} ]; then
      mkdir -p ${PROJECTDIR}
   fi
   echo -e "build.sh copy \033[35m${PROJECT}\033[0m to ${PRIJECTFILE}"
   cp $projectnameforbuild ${PRIJECTFILE}
   echo -e "build.sh copy \033[35m${PROJECT}\033[0m to $homepath/${PROJECT,,}/${PROJECT^^}-BIN"
   mv $projectnameforbuild $homepath/${PROJECT,,}/${PROJECT^^}-BIN
fi