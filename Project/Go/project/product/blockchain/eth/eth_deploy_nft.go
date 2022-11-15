package eth

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/product/blockchain/eth/token/nft_token"
	"dave/public/base"
)

// =====================================================================

func Eth_deploy_nft(nft_uri string, nft_name string) bool {
	auth, err := Eth_transactor()
	if err != nil {
		base.DAVELOG("get auth data failed! %v", err)
		return false
	}
	client := Eth_client_open()
	if client == nil {
		return false
	}

	address, tx, token, err := mytoken.DeployMytoken(auth, client, nft_uri, nft_name, nft_name)
	if err != nil {
		Eth_client_close(client)
		base.DAVELOG("deploy token failed! %v", err)
		return false
	}

	base.DAVELOG("deploy token success! address:%s tx:%v token:%v", address, tx, token)

	Eth_client_close(client)
	return true
}