#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

PRODUCT=$1
TIDY=$2

BUILDMODFILE=`pwd`/${PRODUCT}/go.mod
BUILDSUMFILE=`pwd`/${PRODUCT}/go.sum
PRODUCTMODFILE=`pwd`/../project/go.mod
PRODUCTSUMFILE=`pwd`/../project/go.sum

if [ -f ${BUILDMODFILE} ]; then
   if [ -f ${PRODUCTMODFILE} ]; then
      rm -rf ${PRODUCTMODFILE}
   fi
   cp -rf ${BUILDMODFILE} ${PRODUCTMODFILE}
fi
if [ -f ${BUILDSUMFILE} ]; then
   if [ -f ${PRODUCTSUMFILE} ]; then
      rm -rf ${PRODUCTSUMFILE}
   fi
   cp -rf ${BUILDSUMFILE} ${PRODUCTSUMFILE}
fi

cd ../project

if [ "$TIDY" != "" ]; then
   echo tidy.sh go mod tidy
   go mod tidy
fi

cp -rf ${PRODUCTMODFILE} ${BUILDMODFILE}
cp -rf ${PRODUCTSUMFILE} ${BUILDSUMFILE}