package eth_core

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"github.com/ethereum/go-ethereum/ethclient"
	"dave/public/base"
)

// =====================================================================

func Eth_client_open() (*ethclient.Client) {
	server_url := base.Cfg_get("EthClientURL", "http://127.0.0.1:8545")

	client, err := ethclient.Dial(server_url)
	if err != nil {
		base.DAVELOG("connect server_url:%s failed:%v!", server_url, err)
		return nil
	} else {
		return client
	}
}

func Eth_client_close(client *ethclient.Client) {
	client.Close()
}