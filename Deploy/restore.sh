#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

PROJECTNAME=$1
PROJECT=$2
RESTOREDIR=dave/${PROJECT,,}/config

if [ -d ./config ]; then
   echo restore.sh copy ${REMOVEDIR} directory to ${PROJECTNAME}! 
   docker cp ./config ${PROJECTNAME}:/${RESTOREDIR}
   rm -rf config
fi