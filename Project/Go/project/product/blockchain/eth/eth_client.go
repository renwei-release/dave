package eth

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"github.com/ethereum/go-ethereum/ethclient"
	"dave/public/base"
	"context"
)

// =====================================================================

func Eth_client_open() (*ethclient.Client) {
	server_url := base.Cfg_get("EthClientURL", "http://127.0.0.1:8545")

	client, err := ethclient.Dial(server_url)
	if err != nil {
		base.DAVELOG("connect server_url:%s failed:%v!", server_url, err)

		return nil
	} else {
		networkid, err := client.NetworkID(context.Background())
		if err != nil {
			Eth_client_close(client)
			base.DAVELOG("get netword id failed:%v!", err)

			return nil
		}
		chainid, _ := client.ChainID(context.Background())
		blocknumber, _ := client.BlockNumber(context.Background())

		base.DAVELOG("connect server_url:%s success! NetworkID:%d ChainID:%d blocknumber:%d",
			server_url, networkid, chainid, blocknumber)

		return client
	}
}

func Eth_client_close(client *ethclient.Client) {
	client.Close()
}