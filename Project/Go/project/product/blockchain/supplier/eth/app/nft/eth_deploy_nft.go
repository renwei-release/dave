package eth_nft

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/product/blockchain/supplier/eth/core"
	"dave/product/blockchain/supplier/eth/token/nft_token"
	"dave/public/base"
)

// =====================================================================

func Eth_deploy_nft(nft_data string, nft_name string) (bool, string) {
	auth, err := eth_core.Eth_transactor()
	if err != nil {
		base.DAVELOG("get auth data failed! %v", err)
		return false, ""
	}
	client := eth_core.Eth_client_open()
	if client == nil {
		return false, ""
	}
	defer eth_core.Eth_client_close(client)

	address, tx, token, err := mytoken.DeployMytoken(auth, client, nft_data, nft_name, nft_name)
	if err != nil {
		base.DAVELOG("deploy token failed! %v", err)
		return false, ""
	}

	base.DAVELOG("deploy token(%s) success! address:%s tx:%v token:%v",
		nft_data, address, tx, token)

	uri, err := token.BaseTokenURI(nil)
	if err != nil {
		base.DAVELOG("err:%v", err)
	} else {
		base.DAVELOG("uri:%v", uri)
	}

	byte_address, _ := address.MarshalText()

	return true, string(byte_address)
}