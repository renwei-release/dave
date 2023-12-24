#!/bin/bash
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

# https://eth-docker.net/Usage/QuickStart/

VERSION=2.4.0.0

if [ ! -d ./eth-docker-${VERSION} ]; then
   if [ ! -f v${VERSION}.tar.gz ]; then
      wget https://github.com/eth-educators/eth-docker/archive/refs/tags/v${VERSION}.tar.gz
   fi

   tar -zxvf v${VERSION}.tar.gz
   rm -rf v${VERSION}.tar.gz
fi

cd eth-docker-${VERSION}

./ethd install

./ethd config

./ethd up