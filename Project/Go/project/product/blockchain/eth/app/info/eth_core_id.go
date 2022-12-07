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
	"math/big"
	"context"
)

func _eth_network_id(client *ethclient.Client) *big.Int {
	networkid, err := client.NetworkID(context.TODO())

	if err != nil {
		base.DAVELOG("network id get failed!", networkid)
		return nil
	} else {
		base.DAVELOG("networkid:%d", networkid)
		return networkid
	}
}

func _eth_chain_id(client *ethclient.Client) *big.Int {
	chainid, err := client.ChainID(context.TODO())

	if err != nil {
		base.DAVELOG("chain id get failed!", chainid)
		return nil
	} else {
		base.DAVELOG("chain id:%d", chainid)
		return chainid
	}
}

// =====================================================================

func Eth_network_id() *big.Int {
	client := eth_core.Eth_client_open()
	if client == nil {
		return nil
	}

	networkid := _eth_network_id(client)

	eth_core.Eth_client_close(client)

	return networkid
}

func Eth_chain_id() *big.Int {
	client := eth_core.Eth_client_open()
	if client == nil {
		return nil
	}

	chainid := _eth_chain_id(client)

	eth_core.Eth_client_close(client)

	return chainid
}