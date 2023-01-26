package vsys_core

/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"github.com/virtualeconomy/go-vsys/vsys"
)

var vsys_api = vsys.NewNodeAPI(Vsys_HOST())
var vsys_chain = vsys.NewChain(vsys_api, Vsys_NET())

// =====================================================================

func Vsys_API() *vsys.NodeAPI {
	return vsys_api
}

func Vsys_Chain() *vsys.Chain {
	return vsys_chain
}