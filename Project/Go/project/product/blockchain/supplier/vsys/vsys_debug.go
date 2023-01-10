package VSYS

/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/product/blockchain/supplier/vsys/core"
	"dave/product/blockchain/supplier/vsys/app/nft"
)

func _vsys_deploy_nft() string {
	ret, address := vsys_nft.VSYS_deploy_nft("/ipfs/QmQPXY1Y5zcsku9K57zysk2xJW7THxaV8rXcaiYdMonK1g", "renwei's token")
	if ret == true {
		return "the NFT tokenid is:"+address
	} else {
		return "deploy NFT failed!"
	}
}

// =====================================================================

func VSYS_debug(debug_req string) string {
	debug_rsp := ""

	if debug_req == "deploy" {
		debug_rsp = _vsys_deploy_nft()
	} else if debug_req == "wallet" {
		debug_rsp = vsys_core.VSYS_wallet_address()
	} else {
		debug_rsp = "bad request:"+debug_req
	}

	return debug_rsp
}