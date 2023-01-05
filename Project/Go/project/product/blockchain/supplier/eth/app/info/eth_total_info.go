package eth_info

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/base"
)

// =====================================================================

func Eth_total_info(wallet_address string) {
	networkid := Eth_network_id()
	chainid := Eth_chain_id()
	isContract := Eth_address_type(wallet_address)
	balance := Eth_balance(wallet_address)
	Eth_block(20)
	Eth_block(21)

	address_type := "is contract account"
	if isContract == false {
		address_type = "is external account"
	}

	base.DAVELOG("networkid:%d chainid:%d wallet_address:%s/%s balance:%v",
		networkid, chainid,
		wallet_address, address_type,
		balance)
}