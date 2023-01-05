package VSYS

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/product/blockchain/supplier/vsys/core"
)

var booting_flag = VSYS_booting()

// =====================================================================

func VSYS_booting() bool {
	vsys_core.VSYS_init()
	return true
}