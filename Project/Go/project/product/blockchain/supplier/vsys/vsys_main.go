package vsys

/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/product/blockchain/supplier/vsys/store"
) 

// =====================================================================

func Vsys_init() {
	vsys_store.Vsys_store_init()
}

func Vsys_exit() {
	vsys_store.Vsys_store_exit()
}