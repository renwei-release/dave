#!/bin/bash
#/*
# * Copyright (c) 2024 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

if [ ! -d node ]; then
    echo node not found, please run GENESIS.sh first
    exit 1
fi

chainId=852

echo "请输入当前网络IP: "
read extip
if [ -z "$extip" ]; then
    extip=$(curl -s ifconfig.me)
fi

echo "请输入质押收益者的账号地址"
read account
if [ -z "$account" ]; then
    account=0x964f85BcB7cec7908cd29d485f9f6ce18eBcFCB2
fi

bootnodes=$(cat node/bootnodes)
peer=$(cat node/peer)
#
# peer的值类似/ip4/172.16.0.31/tcp/13000/p2p/16Uiu2HAmAZjsxkJTSCySKMywggH7hA7bLqt1ZF1WtHyBXLeoLJHA
# 其中172.16.0.31为一个IP地址，帮我提取出peer里面的IP地址，且判断这个IP地址是否是本地的IP地址
# 如果是本地的IP地址，就将peer的值改为空
# 如果不是本地的IP地址，就将peer的值不变
# 请将你的代码写在下面的位置
#
peer_ip=$(echo $peer | awk -F'/' '{print $3}')
local_ips=$(ip addr show | grep inet | grep -v inet6 | awk '{print $2}' | cut -d'/' -f1 | grep -v "^127.0.0.1$")
found=0
for ip in $local_ips; do
    if [ "$peer_ip" == "$ip" ]; then
        found=1
        break
    fi
done
if [ "$found" -eq 1 ]; then
    peer=""
fi

echo "正在用 IP:$extip 质押者:$account node:$bootnodes peer:$peer 启动节点，请稍等..."



GETHDATA=./data/gethdata
GETHLOG=./log/geth.log

BEACONDATA=./data/beacondata
BEACONLOG=./log/beacon.log

VALIDATORDATA=./data/validatordata
VALIDATORLOG=./log/validator.log

NODEDATA=./node



chmod a+x ./bin/geth ./bin/beacon-chain-v5.0.3-linux-amd64 ./bin/validator-v5.0.3-linux-amd64

pkill -f geth
pkill -f beacon-chain-v5.0.3-linux-amd64
pkill -f validator-v5.0.3-linux-amd64

sleep 5

./bin/geth --http --http.api eth,net,web3 --ws --ws.api eth,net,web3 --datadir $GETHDATA --syncmode full --networkid $chainId --http.corsdomain "*" --http.vhosts "*" --http.addr "$extip" --nat extip:$extip --bootnodes="$bootnodes" --authrpc.jwtsecret $NODEDATA/jwtsecret >> $GETHLOG 2>&1 &
sleep 10
if [ -z "$peer" ]; then
    ./bin/beacon-chain-v5.0.3-linux-amd64 --datadir $BEACONDATA --min-sync-peers 0 --genesis-state $NODEDATA/genesis.ssz --bootstrap-node= --interop-eth1data-votes --chain-config-file $NODEDATA/config.yml --contract-deployment-block 0 --chain-id $chainId --network-id $chainId --accept-terms-of-use --jwt-secret $NODEDATA/jwtsecret  --suggested-fee-recipient $account --minimum-peers-per-subnet 0 --enable-debug-rpc-endpoints --execution-endpoint http://127.0.0.1:8551 --p2p-static-id >> $BEACONLOG 2>&1 &
else
    ./bin/beacon-chain-v5.0.3-linux-amd64 --datadir $BEACONDATA --min-sync-peers 0 --genesis-state $NODEDATA/genesis.ssz --bootstrap-node= --interop-eth1data-votes --chain-config-file $NODEDATA/config.yml --contract-deployment-block 0 --chain-id $chainId --network-id $chainId --accept-terms-of-use --jwt-secret $NODEDATA/jwtsecret  --suggested-fee-recipient $account --minimum-peers-per-subnet 0 --enable-debug-rpc-endpoints --execution-endpoint http://127.0.0.1:8551 --peer="$peer" >> $BEACONLOG 2>&1 &
fi
./bin/validator-v5.0.3-linux-amd64 --datadir $VALIDATORDATA --accept-terms-of-use --interop-num-validators 64 --chain-config-file $NODEDATA/config.yml >> $VALIDATORLOG 2>&1 &
