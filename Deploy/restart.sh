#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

PROJECT=$1
PROJECTNAME=$2

echo restart.sh restart ${PROJECTNAME} user:${USER} project:${PROJECT} ...
docker restart ${PROJECTNAME}