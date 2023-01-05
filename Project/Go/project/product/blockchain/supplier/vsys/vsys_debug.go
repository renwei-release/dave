package VSYS

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/base"
	"dave/product/blockchain/supplier/vsys/core"
	"dave/product/blockchain/supplier/vsys/app/info"
)

func _connect_account() string {
	acc := vsys_core.VSYS_account()

	base.DAVELOG("address:%v PublicKey:%v PrivateKey:%v AccountSeed:%v",
		acc.Address(),
		acc.PublicKey(), acc.PrivateKey(), acc.AccountSeed())

	return "ok"
}

func _token_info() string {
	if vsys_info.VSYS_token_info("TWscu6rbRF2PEsnY1bRky4aKxxKTzn69WMFLFdLxK") == true {
		return "success"
	} else {
		return "failed"
	}
}

// =====================================================================

func VSYS_debug(debug_req string) string {
	debug_rsp := ""

	if debug_req == "account" {
		debug_rsp = _connect_account()
	} else if debug_req == "info" {
		debug_rsp = _token_info()
	} else {
		debug_rsp = "bad request:"+debug_req
	}

	return debug_rsp
}