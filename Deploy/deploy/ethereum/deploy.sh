#!/bin/bash
#/*
# * Copyright (c) 2023 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

#
# https://eth-docker.net/Usage/QuickStart/
# https://github.com/eth-educators/eth-docker
#

VERSION=2.4.0.0

if [ ! -d ./eth-docker ]; then
	git clone https://github.com/eth-educators/eth-docker.git
	cd eth-docker
	git checkout tags/v${VERSION}
	cd ..
fi

cd eth-docker

./ethd install

./ethd config

./ethd up