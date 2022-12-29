package eth_nft

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"github.com/ethereum/go-ethereum/common"
	"github.com/ethereum/go-ethereum/ethclient"
	"dave/product/blockchain/eth/token/nft_token"
	"dave/product/blockchain/eth/core"
	"dave/public/base"
)

func _eth_search_nft(client *ethclient.Client, token_address string) (bool, string) {
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
	base.DAVELOG("Token address:%v name:%v uri:%v owner:%v", token_address, name, uri, string(address))

	tokenID, err := token.TokensOfOwner(nil, common.HexToAddress(token_address))
	if err != nil {
		base.DAVELOG("TokensOfOwner err:%v", err)
	} else {
		base.DAVELOG("Token address:%s tokenID:%v", token_address, tokenID)
	}

	return true, "OK"
}

// =====================================================================

func Eth_search_nft(address string) (bool, string) {
	client := eth_core.Eth_client_open()
	if client == nil {
		return false, ""
	}
	defer eth_core.Eth_client_close(client)

	return _eth_search_nft(client, address)
}