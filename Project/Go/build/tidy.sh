#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

PROJECT=$1
BUILDMODFILE=`pwd`/${PROJECT}/go.mod
BUILDSUMFILE=`pwd`/${PROJECT}/go.sum
PROJECTMODFILE=`pwd`/../project/go.mod
PROJECTSUMFILE=`pwd`/../project/go.sum

if [ -f ${BUILDMODFILE} ]; then
   cp -rf ${BUILDMODFILE} ${PROJECTMODFILE}
fi
if [ -f ${BUILDSUMFILE} ]; then
   cp -rf ${BUILDSUMFILE} ${PROJECTSUMFILE}
fi

cd ../project
echo tidy.sh go mod tidy
go mod tidy

cp -rf ${PROJECTMODFILE} ${BUILDMODFILE}
cp -rf ${PROJECTSUMFILE} ${BUILDSUMFILE}