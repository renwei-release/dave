#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

homepath=$(cd `dirname $0`; pwd)

PROJECT=$1
TAGS=$2

cd ../project

GOOS=linux GOARCH=amd64 go clean .

echo -e "clean.sh clean \033[35m${PROJECT}\033[0m"
