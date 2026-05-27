#!/bin/bash
#/*
# * Copyright (c) 2024 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

home_dir=$(eval echo ~${SUDO_USER})

echo the home: $home_dir

source $home_dir/.bashrc

cd LayerZero-v2/packages/layerzero-v2/evm/protocol

npm install

forge build

echo "请输入合约部署链的RPC地址: "
read rpc_url
if [ -z "$rpc_url" ]; then
    rpc_url=https://eth.jegotrip.net
fi

echo "请输入钱包地址的私钥: "
read private_key
if [ -z "$private_key" ]; then
    private_key=0x0
fi

echo "请输入要部署的智能合约的名字: "
read contract_name
if [ -z "$contract_name" ]; then
    contract_name=EndpointV2
fi

echo "请输入要智能合约的构造参数: "
read contract_args
if [ -z "$contract_args" ]; then
    contract_args="852 0xc201eE408DB9DA5867643376DE27Fb10163E9403"
fi

forge create --rpc-url $rpc_url --private-key $private_key $contract_name --constructor-args $contract_args