package vsys

/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/base"
	"dave/public/tools"
	"dave/product/blockchain/supplier/vsys/core"
	"dave/product/blockchain/supplier/vsys/store"
	"dave/product/blockchain/supplier/vsys/app/nft"
	"dave/product/blockchain/supplier/vsys/app/info"
)

func _vsys_deploy_nft() string {
	ret, address := vsys_nft.Vsys_deploy_nft("/ipfs/QmQPXY1Y5zcsku9K57zysk2xJW7THxaV8rXcaiYdMonK1g", "renwei's token")
	if ret == true {
		return "the NFT tokenid is:"+address
	} else {
		return "deploy NFT failed!"
	}
}

func _vsys_add_voucher() string {
	err := vsys_store.Vsys_store_voucher_add(tools.T_rand(), "bbbb", tools.T_rand())
	if err != nil {
		base.DAVELOG("err:%v", err)
		return "failed"
	}

	return "OK"
}

// =====================================================================

func Vsys_debug(debug_req string) string {
	debug_rsp := ""

	if debug_req == "deploy" {
		debug_rsp = _vsys_deploy_nft()
	} else if debug_req == "wallet" {
		debug_rsp = vsys_core.Vsys_new_wallet()
	} else if debug_req == "info" {
		debug_rsp = vsys_info.Vsys_info_total()
	} else if debug_req == "add_voucher" {
		debug_rsp = _vsys_add_voucher()
	} else {
		debug_rsp = "bad request:"+debug_req
	}

	return debug_rsp
}