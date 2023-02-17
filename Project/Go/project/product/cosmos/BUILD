#!/bin/bash
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

PROJECT=${PWD##*/}
PROJECTDIR=`pwd`/../../../../../Deploy/deploy/${PROJECT,,}/file_system/project
PRIJECTBIN=COSMOS-BIN
File=$(basename $0)

chmod a+x gaia-BUILD
./gaia-BUILD ${PROJECTDIR}

echo ${File} copy ${PRIJECTBIN} to ${PROJECTDIR}
chmod a+x ${PRIJECTBIN}
cp ${PRIJECTBIN} ${PROJECTDIR}