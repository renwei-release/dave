#!/bin/bash
#/*
# * Copyright (c) 2022 Chenxiaomin
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

PROJECT=${PWD##*/}

PROJECTNAME=${PROJECT}-log

cd ../../
chmod a+x *.sh
./debug.sh $PROJECTNAME $PROJECT