#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

PROJECT=$1
LEVEL=$2
PROCESSOR=`cat /proc/cpuinfo| grep "processor"| wc -l`
PROJECT=${PROJECT^^}

make NAME_PRODUCT=${PROJECT} LEVEL_PRODUCT=${LEVEL} pre_build
make -j${PROCESSOR} NAME_PRODUCT=${PROJECT} LEVEL_PRODUCT=${LEVEL}
make NAME_PRODUCT=${PROJECT} LEVEL_PRODUCT=${LEVEL} end_build