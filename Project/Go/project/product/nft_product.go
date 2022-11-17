//go:build __DAVE_PRODUCT_NFT__
// +build __DAVE_PRODUCT_NFT__

package product

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/product/nft"
)

// =====================================================================

func Product_init() {
	nft.Dave_product_init()
}

func Product_exit() {
	nft.Dave_product_exit()
}
