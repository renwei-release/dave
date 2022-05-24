//go:build __DAVE_PRODUCT_BASE__
// +build __DAVE_PRODUCT_BASE__

package product

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/product/base"
)

// =====================================================================

func Product_init() {
	base.Dave_product_init()
}

func Product_exit() {
	base.Dave_product_exit()
}
