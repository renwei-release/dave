#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

homepath=$(cd `dirname $0`; pwd)
PROJECT=$1
if [ "$PROJECT" == "" ]; then
   PROJECT=dave
fi
projectnameforbuild=projectname${PROJECT}

python3 ../../../Tools/refresh_version/refresh_version.py "../../../" ${PROJECT^^}

if [ -f $PROJECT ]; then
   rm -rf $PROJECT
fi

cd ../project

GOOS=linux GOARCH=amd64 go build -tags __DAVE_PRODUCT_${PROJECT^^}__ -o $projectnameforbuild dave_main.go

if [ -f $projectnameforbuild ]; then
   PROJECTDIR=../../../Deploy/deploy/${PROJECT,,}/project
   PRIJECTFILE=${PROJECTDIR}/${PROJECT^^}-BIN

   if [ ! -d ${PROJECTDIR} ]; then
      mkdir -p ${PROJECTDIR}
   fi
   echo build.sh copy $projectnameforbuild to ${PRIJECTFILE}
   cp $projectnameforbuild ${PRIJECTFILE}
   echo build.sh copy $projectnameforbuild to $homepath/${PROJECT,,}/${PROJECT^^}-BIN
   mv $projectnameforbuild $homepath/${PROJECT,,}/${PROJECT^^}-BIN
fi