#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

PROJECT=${PWD##*/}
if [ "$USER" == "root" ]; then
   PROJECTNAME=${PROJECT}
else
   PROJECTNAME=${PROJECT}-${USER}
fi

cd ../../
chmod a+x *.sh
./debug.sh $PROJECTNAME