package blockchain

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/product/blockchain/supplier/eth"
	"dave/product/blockchain/supplier/vsys"
	"dave/product/blockchain/supplier/polygon"
)

// =====================================================================

func blockchain_debug(debug_req string) string {
	debug_rsp := ""

	if (len(debug_req) > 3) && (debug_req[0:3] == "eth") {
		debug_rsp = eth.Eth_debug(debug_req[4:])
	} else if (len(debug_req) > 4) && (debug_req[0:4] == "vsys") {
		debug_rsp = VSYS.VSYS_debug(debug_req[5:])
	} else if (len(debug_req) > 4) && (debug_req[0:2] == "pg") {
		debug_rsp = pg.PG_debug(debug_req[3:])
	} else {
		debug_rsp = "bad request:"+debug_req
	}

	return debug_rsp
}