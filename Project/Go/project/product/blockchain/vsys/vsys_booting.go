package VSYS

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/product/blockchain/vsys/core"
)

// =====================================================================

func VSYS_booting() {
	vsys_core.VSYS_init()
}