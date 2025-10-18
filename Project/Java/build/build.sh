#!/bin/bash
#/*
# * Copyright (c) 2025 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
homedir=$(cd `dirname $0`; pwd)

PRODUCT=$1

python3 ../../../Tools/refresh_version/refresh_version.py "../../../" ${PRODUCT}

cd ${homedir}/../project/public/base

mvn -DskipTests install || exit 1

cd ${homedir}/../project/product/${PRODUCT}

mvn -U clean compile package || exit 1

if [ ! -d "../../../../../Deploy/deploy/$PRODUCT/file_system/project" ]; then
    mkdir -p ../../../../../Deploy/deploy/$PRODUCT/file_system/project
fi
cp ${homedir}/../project/product/${PRODUCT}/target/${PRODUCT^^}.jar ../../../../../Deploy/deploy/$PRODUCT/file_system/project