#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

homepath=$(cd `dirname $0`; pwd)

PRODUCT=$1
TAGS=$2
TIDY=$3
ARCH=`arch`

if [ "$PRODUCT" == "" ]; then
   PRODUCT=dave
fi
projectnameforbuild=projectname${PRODUCT}

python3 ../../../Tools/refresh_version/refresh_version.py "../../../" ${PRODUCT^^}

if [ -f tidy.sh ]; then
   chmod a+x tidy.sh
   ./tidy.sh ${PRODUCT} ${TIDY}
fi

if [ -f $PRODUCT ]; then
   rm -rf $PRODUCT
fi

if [ "$ARCH" == "x86_64" ]; then
   GOARCH=amd64
elif [ "$ARCH" == "aarch64" ]; then
   GOARCH=arm64
else
   echo Please define the GOARCH!
fi

cd ../project

GOOS=linux GOARCH=${GOARCH} go build -gcflags=all="-N -l" -tags "${TAGS} __DAVE_PRODUCT_${PRODUCT^^}__" -o $projectnameforbuild dave_main.go

if [ -f $projectnameforbuild ]; then
   PRODUCTDIR=../../../Deploy/deploy/${PRODUCT,,}/file_system/project
   PRIJECTFILE=${PRODUCTDIR}/${PRODUCT^^}-BIN

   if [ ! -d ${PRODUCTDIR} ]; then
      mkdir -p ${PRODUCTDIR}
   fi
   echo -e "build.sh copy \033[35m${PRODUCT}\033[0m to ${PRIJECTFILE}"
   cp $projectnameforbuild ${PRIJECTFILE}
   echo -e "build.sh copy \033[35m${PRODUCT}\033[0m to $homepath/${PRODUCT,,}/${PRODUCT^^}-BIN"
   mv $projectnameforbuild $homepath/${PRODUCT,,}/${PRODUCT^^}-BIN
fi