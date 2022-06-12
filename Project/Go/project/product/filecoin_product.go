//go:build __DAVE_PRODUCT_FILECOIN__
// +build __DAVE_PRODUCT_FILECOIN__

package product

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/product/filecoin"
)

// =====================================================================

func Product_init() {
	filecoin.Dave_product_init()
}

func Product_exit() {
	filecoin.Dave_product_exit()
}
