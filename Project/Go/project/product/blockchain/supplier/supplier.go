package supplier

/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/product/blockchain/supplier/vsys"
)

// =====================================================================

func Supplier_init() {
	vsys.Vsys_init()
}

func Supplier_exit() {
	vsys.Vsys_exit()
}