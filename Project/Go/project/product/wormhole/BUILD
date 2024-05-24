#!/bin/bash
#/*
# * Copyright (c) 2024 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

VERSION=2.24.2

if [ ! -f v${VERSION}.tar.gz ]; then
   wget https://github.com/wormhole-foundation/wormhole/archive/refs/tags/v${VERSION}.tar.gz
fi

if [ ! -d ./wormhole-${VERSION} ]; then
   tar -zxvf v${VERSION}.tar.gz
fi

cd wormhole-${VERSION}

#
cd node
sed -i '/github.com\/algorand\/go-codec\/codec v1.1.8/d' go.sum
sed -i '/github.com\/google\/flatbuffers v1.12.0 h1/d' go.sum
go clean -modcache
export GONOSUMDB="*" && go mod tidy
export GONOSUMDB="*" && go get github.com/dgraph-io/badger/v3/table@v3.2103.1
cd ../
#

export GONOSUMDB="*" && make node