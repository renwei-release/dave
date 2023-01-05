package eth_info

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"github.com/ethereum/go-ethereum/ethclient"
	"github.com/ethereum/go-ethereum/common"
	"dave/product/blockchain/supplier/eth/core"
	"dave/public/base"
	"math/big"
	"math"
	"context"
)

func _eth_balance(client *ethclient.Client, wallet_address string) *big.Float {
	balance, err := client.BalanceAt(context.TODO(), common.HexToAddress(wallet_address), nil)

	if err != nil {
		base.DAVELOG("wallet_address:%s get balance failed!", wallet_address)
		return nil
	} else {
		fbalance := new(big.Float)
		fbalance.SetString(balance.String())
		ethValue := new(big.Float).Quo(fbalance, big.NewFloat(math.Pow10(18)))

		base.DAVELOG("wallet_address:%s balance:%v", wallet_address, ethValue)
		return ethValue
	}
}

// =====================================================================

func Eth_balance(wallet_address string) *big.Float {
	client := eth_core.Eth_client_open()
	if client == nil {
		return nil
	}

	balance := _eth_balance(client, wallet_address)

	eth_core.Eth_client_close(client)

	return balance
}