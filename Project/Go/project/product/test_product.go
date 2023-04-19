//go:build __DAVE_PRODUCT_TEST__
// +build __DAVE_PRODUCT_TEST__

package product

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/product/test"
)

// =====================================================================

func Product_init() {
	test.Dave_product_init()
}

func Product_exit() {
	test.Dave_product_exit()
}

func Product_cfg() (int, string) {
	return 0, "Coroutine Inner Loop"
}