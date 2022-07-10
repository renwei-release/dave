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
   PROJECT=sync
fi
PROJECTDIR=$homepath/../../Project/C/build/linux/${PROJECT}
BINNAME=${PROJECT^^}-BIN
BINFILE=${PROJECTDIR}/${BINNAME}
WORKDIR=$homepath/project

echo PROJECT:${PROJECT}
echo PROJECT DIR:${PROJECTDIR}
echo BIN NAME:${BINNAME}
echo BIN FILE:${BINFILE}
echo WORK DIR:${WORKDIR}
echo
echo

build_valgrind()
{
   cd ${homepath}

   if [ ! -f ./setup/bin/valgrind ]; then
      echo build valgrind ...
      echo
      chmod a+x ./build-valgrind
      ./build-valgrind > /dev/null
   fi
}

build_project()
{
   cd ${homepath}

   if [ -f ${BINFILE} ]; then
      echo copy ${BINFILE}
      echo
      cp ${BINFILE} ${WORKDIR}/${BINNAME}
   fi

   if [ ! -f ${WORKDIR}/${BINNAME} ]; then
      echo build ${BINNAME} ...
      echo
      cd ${PROJECTDIR}
      chmod a+x clean build
      ./clean > /dev/null
      ./build > /dev/null
      cd ${homepath}
      cp ${BINFILE} ${WORKDIR}/${BINNAME}
   fi
}

run_helgrind()
{
   cd ${homepath}

   if [ -f ${WORKDIR}/${BINNAME} ]; then
      chmod a+x ${WORKDIR}/${BINNAME}
      sudo ./setup/bin/valgrind --tool=helgrind --log-file=${WORKDIR}/${BINNAME}.txt ${WORKDIR}/${BINNAME}
   else
      echo File ${WORKDIR}/${BINNAME} does not exist, nothing is done!
   fi
}

mkdir -p ${WORKDIR}

build_valgrind
build_project
run_helgrind