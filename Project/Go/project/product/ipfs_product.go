//go:build __DAVE_PRODUCT_IPFS__
// +build __DAVE_PRODUCT_IPFS__

package product

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/product/ipfs"
)

// =====================================================================

func Product_init() {
	ipfs.Dave_product_init()
}

func Product_exit() {
	ipfs.Dave_product_exit()
}
