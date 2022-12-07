package eth_info

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"github.com/ethereum/go-ethereum/ethclient"
	"dave/product/blockchain/eth/core"
	"dave/public/base"
	"context"
	"math/big"
)

func _eth_block(client *ethclient.Client, blocknumber *big.Int) {
	block, err := client.BlockByNumber(context.Background(), blocknumber)
	if err != nil {
	  base.DAVELOG("get the block:%v failed:%v!", blocknumber, err)
	  return
	}

	base.DAVELOG("blocknumber:%v head hash:%v number:%v ParentHash:%v MixDigest:%v GasLimit:%v",
		blocknumber,
		block.Hash().Hex(),
		block.Number().Uint64(),
		block.ParentHash().Hex(),
		block.MixDigest().Hex(),
		block.GasLimit())
}

// =====================================================================

func Eth_block(blocknumber int64) {
	client := eth_core.Eth_client_open()
	if client == nil {
		return
	}

	big_blockNumber := big.NewInt(blocknumber)

	_eth_block(client, big_blockNumber)

	eth_core.Eth_client_close(client)
}