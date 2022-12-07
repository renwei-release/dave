package eth_nft

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"github.com/ethereum/go-ethereum/common"
	"dave/product/blockchain/eth/token/nft_token"
	"dave/product/blockchain/eth/core"
	"dave/public/base"
)

func _eth_search_nft_of_token_address(token_address string) (bool, string) {
	client := eth_core.Eth_client_open()
	if client == nil {
		return false, ""
	}

	token, err := mytoken.NewMytoken(common.HexToAddress(token_address), client)
	if err != nil {
		base.DAVELOG("Failed to instantiate a Token contract: %v", err)
	}

	name, err := token.Name(nil)
	if err != nil {
		base.DAVELOG("Failed to retrieve token name: %v", err)
	}
	uri, err := token.BaseTokenURI(nil)
	if err != nil {
		base.DAVELOG("Failed to retrieve token uri: %v", err)
	}
	owner, err := token.Owner(nil)
	if err != nil {
		base.DAVELOG("Failed to retrieve token owner: %v", err)
	}
	address, _ := owner.MarshalText()

	base.DAVELOG("Token name:%v uri:%v owner:%v", name, uri, string(address))

	eth_core.Eth_client_close(client)

	return true, "OK"
}

// =====================================================================

func Eth_search_nft(is_token_address bool, address string) (bool, string) {
	if is_token_address == true {
		return _eth_search_nft_of_token_address(address)
	} else {
		return false, ""
	}
}