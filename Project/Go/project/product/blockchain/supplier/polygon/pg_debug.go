package pg

/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/product/blockchain/supplier/polygon/core"
)

// =====================================================================

func PG_debug(debug_req string) string {
	debug_rsp := ""

	if debug_req == "chain" {
		debug_rsp = pg_core.PG_chainId()
	} else {
		debug_rsp = "bad request:"+debug_req
	}

	return debug_rsp
}