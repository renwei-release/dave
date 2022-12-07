package eth_info

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"github.com/ethereum/go-ethereum/common"
	"dave/product/blockchain/eth/core"
	"dave/public/base"
	"context"
)

// =====================================================================

func Eth_address_type(address string) bool {
	client := eth_core.Eth_client_open()
	if client == nil {
		base.DAVELOG("client open failed!")
		return false
	}

	bytecode, err := client.CodeAt(context.Background(), common.HexToAddress(address), nil) // nil is latest block
    if err != nil {
		base.DAVELOG("client codeat failed:%v!", err)
		return false
    }

	base.DAVELOG("address:%s bytecode:%v", address, bytecode)

	isContract := false
    if len(bytecode) > 0 {
		isContract = true
	}

	eth_core.Eth_client_close(client)

	return isContract
}