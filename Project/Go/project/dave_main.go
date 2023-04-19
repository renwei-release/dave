package main
/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/base"
	"dave/product"
	"strings"
)

// =====================================================================

func main() {
	_, workmode := product.Product_cfg()

	base.Dave_go_init("", workmode, "", product.Product_init, product.Product_exit)
	if find := strings.Contains(workmode, "Outer"); find {
		base.Dave_go_running()
	}
	base.Dave_go_exit()
}