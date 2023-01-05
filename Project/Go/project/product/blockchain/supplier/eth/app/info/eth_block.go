package eth_info

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"context"
	"math/big"
	"reflect"
	"fmt"
	"regexp"
	"strings"
	"strconv"

	"github.com/ethereum/go-ethereum/core/types"
	"github.com/ethereum/go-ethereum/ethclient"
	"github.com/ethereum/go-ethereum/common"

	"dave/public/base"
	"dave/product/blockchain/supplier/eth/core"
)

func _eth_block_head(block *types.Block) {
	base.DAVELOG("blocknumber:%v coinbase:%v StateRootHash:%v TxHash:%v ReceiptHash:%v head hash:%v number:%v ParentHash:%v MixDigest:%v GasLimit:%v Extra:%v",
		block.NumberU64(),
		block.Coinbase().Hex(),
		block.Root().Hex(),
		block.TxHash().Hex(),
		block.ReceiptHash().Hex(),
		block.Hash().Hex(),
		block.Number().Uint64(),
		block.ParentHash().Hex(),
		block.MixDigest().Hex(),
		block.GasLimit(),
		block.Extra())
}

func _eth_block_transactions(block *types.Block) {

	for _, tx := range block.Transactions() {
		if tx != nil {
			base.DAVELOG("tx:%+v", tx)

			base.DAVELOG("Hash:%v", tx.Hash().Hex())
			base.DAVELOG("Value:%v", tx.Value().String())
			base.DAVELOG("Gas:%v", tx.Gas())
			base.DAVELOG("GasPrice:%v", tx.GasPrice().Uint64())
			base.DAVELOG("Nonce:%v", tx.Nonce())
			base.DAVELOG("Data:%v", tx.Data())

			to := tx.To()
			if to != nil {
				base.DAVELOG("To:%v", to.Hex())
			}

			cache_value := fmt.Sprintf("%v", reflect.ValueOf(*tx).FieldByName("from"))
			compileRegex := regexp.MustCompile(`\[(.*?)\]`)
			from_value := compileRegex.FindStringSubmatch(cache_value)
			hex_array := strings.Fields(from_value[1])
			var from common.Address
			for i := range hex_array {
				data, err := strconv.Atoi(hex_array[i])
				if err != nil {
					base.DAVELOG("on %d has invalid digital:%v", i, hex_array)
					break
				}
				from[i] = byte(data)
			}
			base.DAVELOG("From:%v", from.Hex())
		}
	}

}

func _eth_block(client *ethclient.Client, blocknumber *big.Int) {
	block, err := client.BlockByNumber(context.Background(), blocknumber)
	if err != nil {
		base.DAVELOG("get the block:%v failed:%v!", blocknumber, err)
		return
	}

	_eth_block_head(block)

	_eth_block_transactions(block)
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
