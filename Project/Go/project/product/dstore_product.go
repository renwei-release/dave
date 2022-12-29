//go:build __DAVE_PRODUCT_DSTORE__
// +build __DAVE_PRODUCT_DSTORE__

package product

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/product/dstore"
)

// =====================================================================

func Product_init() {
	dstore.Dave_product_init()
}

func Product_exit() {
	dstore.Dave_product_exit()
}
