package main
/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/dave/base"
	"dave/product"
)

func main() {
	base.Dave_go_init(product.Product_init, product.Product_exit)
	base.Dave_go_running()
	base.Dave_go_exit()
}