#!/bin/bash
#/*
# * Copyright (c) 2024 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

chainId=852

echo "请输入节点RPC的IP: "
read rpcurl
if [ -z "$rpcurl" ]; then
    rpcurl=eth.jegotrip.net
fi

echo "请输入区块链节点IP: "
read noteip
if [ -z "$noteip" ]; then
    noteip=$rpcurl
fi

echo "请输入外部可访问的IP: "
read extip
if [ -z "$extip" ]; then
    extip=$(curl -s ifconfig.me)
fi

version=6.5.0

if [ ! -f v${version}-beta.tar.gz ]; then
    wget https://github.com/blockscout/blockscout/archive/refs/tags/v${version}-beta.tar.gz
fi
if [ ! -d ./blockscout-${version}-beta ]; then
    tar -zxvf v${version}-beta.tar.gz
fi

if [ ! -f docker-compose ]; then
    curl -SL https://github.com/docker/compose/releases/download/v2.20.2/docker-compose-linux-x86_64 -o docker-compose
fi
chmod a+x docker-compose

cp ./cfg/docker-compose.yml ./blockscout-${version}-beta/docker-compose/docker-compose.yml
cp ./cfg/nginx.yml ./blockscout-${version}-beta/docker-compose/services/nginx.yml
cp ./cfg/common-frontend.env ./blockscout-${version}-beta/docker-compose/envs/common-frontend.env

cd blockscout-${version}-beta/docker-compose

echo "正在用RPC：$rpcurl 节点IP：$noteip 可访问IP：$extip 配置区块链流量器，请稍等..."

sed -i "s/__CHAIN_RPC__/$rpcurl/g" docker-compose.yml
sed -i "s/__CHAIN_ID__/$chainId/g" docker-compose.yml
sed -i "s/__NODE_IP__/$noteip/g" ./services/nginx.yml
sed -i "s/__NODE_IP__/$noteip/g" ./envs/common-frontend.env
sed -i "s/__CHAIN_ID__/$chainId/g" ./envs/common-frontend.env
sed -i "s/__EXTERNAL_IP__/$extip/g" ./envs/common-frontend.env

if [ ! -f /usr/bin/docker-compose ]; then
    sudo apt update
    sudo apt install -y docker-compose
fi

echo setup blockscout access to $rpcurl and $extip success!