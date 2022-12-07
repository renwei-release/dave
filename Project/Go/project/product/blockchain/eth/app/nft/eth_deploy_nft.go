package eth_nft

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/product/blockchain/eth/core"
	"dave/product/blockchain/eth/token/nft_token"
	"dave/public/base"
)

// =====================================================================

func Eth_deploy_nft(nft_uri string, nft_name string) (bool, string) {
	auth, err := eth_core.Eth_transactor()
	if err != nil {
		base.DAVELOG("get auth data failed! %v", err)
		return false, ""
	}
	client := eth_core.Eth_client_open()
	if client == nil {
		return false, ""
	}

	address, tx, token, err := mytoken.DeployMytoken(auth, client, nft_uri, nft_name, nft_name)
	if err != nil {
		eth_core.Eth_client_close(client)
		base.DAVELOG("deploy token failed! %v", err)
		return false, ""
	}

	base.DAVELOG("deploy token success! address:%s tx:%v token:%v", address, tx, token)

	eth_core.Eth_client_close(client)

	byte_address, _ := address.MarshalText()

	return true, string(byte_address)
}