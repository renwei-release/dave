#!/bin/bash
#/*
# * ================================================================================
# * (c) Copyright 2021 Renwei All rights reserved.
# * --------------------------------------------------------------------------------
# * 2021.10.12.
# * ================================================================================
# */

PROJECT=${PWD##*/}
if [ "$USER" == "root" ]; then
   PROJECTNAME=${PROJECT}
else
   PROJECTNAME=${PROJECT}-${USER}
fi

cd ../../
chmod a+x *.sh
./debug.sh $PROJECTNAME $PROJECT