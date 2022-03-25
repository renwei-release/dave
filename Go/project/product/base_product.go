// +build __DAVE_PRODUCT_BASE__

package product
/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import "dave/product/base"

// =====================================================================

func Product_init() {
	base.Main_init()
}

func Product_exit() {
	base.Main_exit()
}