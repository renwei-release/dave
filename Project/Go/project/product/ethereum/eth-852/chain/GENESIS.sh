#!/bin/bash
#/*
# * Copyright (c) 2024 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

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
account=$(echo "$account" | sed 's/^0x//')

echo "请输入提款质押智能合约的账号地址"
read contract_account
if [ -z "$contract_account" ]; then
  contract_account=0xdbBeA4A21Cc32838B615Fd433f9935bF5940052A
fi
contract_account=$(echo "$contract_account" | sed 's/^0x//')


echo "请输入质押收益者的余额"
read balance
if [ -z "$balance" ]; then
  balance=9000000000000000000000000000
fi

echo "正在用 链的ID:$chainId IP:$extip 质押者:$account 资金：$balance 启动创世节点，请稍等..."



if [ -d "node" ]; then
  rm -rf node
fi
mkdir node
if [ -d "data" ]; then
  rm -rf data
fi
mkdir data
if [ -d "log" ]; then
  rm -rf log
fi
mkdir log

GETHDATA=./data/gethdata
GETHLOG=./log/geth.log

BEACONDATA=./data/beacondata
BEACONLOG=./log/beacon.log

VALIDATORDATA=./data/validatordata
VALIDATORLOG=./log/validator.log

GENESISNODEDATA=./genesis_node
if [ -d $GENESISNODEDATA ]; then
  rm -rf $GENESISNODEDATA
fi
mkdir $GENESISNODEDATA
NODEDATA=./node
if [ -d $NODEDATA ]; then
  rm -rf $NODEDATA
fi
mkdir $NODEDATA



cp ./cfg/genesis.json $GENESISNODEDATA/genesis.json
cp ./cfg/config.yml $GENESISNODEDATA/config.yml

sed -i "s/\"chainId\": __CHAIN_ID__,/\"chainId\": $chainId,/g" $GENESISNODEDATA/genesis.json
sed -i "s/__PLEDGER_extraData_ACCOUNT__/$account/g" $GENESISNODEDATA/genesis.json
sed -i "s/__PLEDGER_ACCOUNT__/\"$account\"/g" $GENESISNODEDATA/genesis.json
sed -i "s/__PLEDGER_BALANCE__/\"$balance\"/g" $GENESISNODEDATA/genesis.json
sed -i "s/__CONTRACT_ACCOUNT__/\"$contract_account\"/g" $GENESISNODEDATA/genesis.json
sed -i "s/__CONTRACT_ACCOUNT__/$contract_account/g" $GENESISNODEDATA/config.yml



#
# prysmctl-v5.0.3-linux-amd64 testnet
# 这里使用testnet的原因是mainnet的配置是不允许带入节点参数的，不方便调试。
#

chmod a+x ./bin/prysmctl-v5.0.3-linux-amd64 ./bin/geth ./bin/beacon-chain-v5.0.3-linux-amd64 ./bin/validator-v5.0.3-linux-amd64

pkill -f geth
pkill -f beacon-chain-v5.0.3-linux-amd64
pkill -f validator-v5.0.3-linux-amd64

sleep 5

./bin/prysmctl-v5.0.3-linux-amd64 testnet generate-genesis --fork capella --num-validators 64 --genesis-time-delay 60 --chain-config-file $GENESISNODEDATA/config.yml --geth-genesis-json-in $GENESISNODEDATA/genesis.json  --geth-genesis-json-out $GENESISNODEDATA/genesis.json --output-ssz $GENESISNODEDATA/genesis.ssz
./bin/geth --datadir=$GETHDATA --networkid $chainId init $GENESISNODEDATA/genesis.json
./bin/geth --http --http.api eth,net,web3 --ws --ws.api eth,net,web3 --datadir $GETHDATA --syncmode full --networkid $chainId --http.corsdomain "*" --http.vhosts "*" --http.addr "$extip" --nat extip:$extip >> $GETHLOG 2>&1 &
sleep 20
./bin/beacon-chain-v5.0.3-linux-amd64 --datadir $BEACONDATA --min-sync-peers 0 --genesis-state $GENESISNODEDATA/genesis.ssz --bootstrap-node= --interop-eth1data-votes --chain-config-file $GENESISNODEDATA/config.yml --contract-deployment-block 0 --deposit-contract 0x$contract_account --chain-id $chainId --accept-terms-of-use --jwt-secret $GETHDATA/geth/jwtsecret  --suggested-fee-recipient 0x$account --minimum-peers-per-subnet 0 --enable-debug-rpc-endpoints --execution-endpoint http://127.0.0.1:8551 --p2p-static-id >> $BEACONLOG 2>&1 &
./bin/validator-v5.0.3-linux-amd64 --datadir $VALIDATORDATA --accept-terms-of-use --beacon-rpc-provider=127.0.0.1:4000 --interop-num-validators 64 --chain-config-file $GENESISNODEDATA/config.yml >> $VALIDATORLOG 2>&1 &



sleep 20
cp $GETHDATA/geth/jwtsecret $GENESISNODEDATA/jwtsecret

output=$(curl -s localhost:8080/p2p)
peer=$(echo "$output" | grep "/ip4/" | awk -F',' '{print $2}')
if [ -f $GENESISNODEDATA/peer ]; then
  rm -f $GENESISNODEDATA/peer
fi
echo $peer > $GENESISNODEDATA/peer

bootnodes=$(./bin/geth attach --exec "admin.nodeInfo.enode" ipc:./data/gethdata/geth.ipc)
bootnodes=${bootnodes#\"}
bootnodes=${bootnodes%\"}
if [ -f $GENESISNODEDATA/bootnodes ]; then
  rm -f $GENESISNODEDATA/bootnodes
fi
echo $bootnodes > $GENESISNODEDATA/bootnodes

cp -rf $GENESISNODEDATA/* $NODEDATA/



echo "启动成功，请60秒之后在钱包访问，访问地址：http://$extip:8545"