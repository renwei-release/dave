#!/bin/bash
#/*
# * Copyright (c) 2024 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

NODEDATA=./node

if [ ! -d $NODEDATA ]; then
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

bootnodes=$(cat $NODEDATA/bootnodes)
peer=$(cat $NODEDATA/peer)

echo "正在用 IP:$extip 质押者:$account node:$bootnodes peer:$peer 启动节点，请稍等..."



if [ -d ./data ]; then
  rm -rf data
fi
mkdir data
if [ -d ./log ]; then
  rm -rf log
fi
mkdir log

GETHDATA=./data/gethdata
GETHLOG=./log/geth.log

BEACONDATA=./data/beacondata
BEACONLOG=./log/beacon.log

VALIDATORDATA=./data/validatordata
VALIDATORLOG=./log/validator.log

MYNODEDATA=./my_node
if [ -d $MYNODEDATA ]; then
  rm -rf $MYNODEDATA
fi
mkdir $MYNODEDATA



chmod a+x ./bin/geth ./bin/beacon-chain-v5.0.3-linux-amd64 ./bin/validator-v5.0.3-linux-amd64

pkill -f geth
pkill -f beacon-chain-v5.0.3-linux-amd64
pkill -f validator-v5.0.3-linux-amd64

sleep 5

./bin/geth --datadir=$GETHDATA --networkid $chainId --authrpc.jwtsecret $NODEDATA/jwtsecret init $NODEDATA/genesis.json
./bin/geth --http --http.api eth,net,web3 --ws --ws.api eth,net,web3 --datadir $GETHDATA --syncmode full --networkid $chainId --http.corsdomain "*" --http.vhosts "*" --http.addr "$extip" --nat extip:$extip --bootnodes="$bootnodes" --authrpc.jwtsecret $NODEDATA/jwtsecret >> $GETHLOG 2>&1 &
sleep 20
./bin/beacon-chain-v5.0.3-linux-amd64 --datadir $BEACONDATA --min-sync-peers 0 --genesis-state $NODEDATA/genesis.ssz --bootstrap-node= --interop-eth1data-votes --chain-config-file $NODEDATA/config.yml --contract-deployment-block 0 --chain-id $chainId --accept-terms-of-use --jwt-secret $NODEDATA/jwtsecret  --suggested-fee-recipient $account --minimum-peers-per-subnet 0 --enable-debug-rpc-endpoints --execution-endpoint http://127.0.0.1:8551 --peer="$peer" >> $BEACONLOG 2>&1 &
./bin/validator-v5.0.3-linux-amd64 --datadir $VALIDATORDATA --accept-terms-of-use --interop-num-validators 64 --chain-config-file $NODEDATA/config.yml >> $VALIDATORLOG 2>&1 &



sleep 20
if [ -d $MYNODEDATA ]; then
  rm -rf $MYNODEDATA
fi
mkdir $MYNODEDATA
cp $NODEDATA/config.yml $MYNODEDATA/config.yml
cp $NODEDATA/genesis.json $MYNODEDATA/genesis.json
cp $NODEDATA/genesis.ssz $MYNODEDATA/genesis.ssz
cp $NODEDATA/jwtsecret $MYNODEDATA/jwtsecret

output=$(curl -s localhost:8080/p2p)
peer=$(echo "$output" | grep "/ip4/" | awk -F',' '{print $2}')
if [ -f $MYNODEDATA/peer ]; then
  rm -f $MYNODEDATA/peer
fi
echo $peer > $MYNODEDATA/peer

bootnodes=$(./bin/geth attach --exec "admin.nodeInfo.enode" ipc:./data/gethdata/geth.ipc)
bootnodes=${bootnodes#\"}
bootnodes=${bootnodes%\"}
if [ -f $MYNODEDATA/bootnodes ]; then
  rm -f $MYNODEDATA/bootnodes
fi
echo $bootnodes > $MYNODEDATA/bootnodes



echo "启动成功，请60秒之后在钱包访问，访问地址：http://$extip:8545"