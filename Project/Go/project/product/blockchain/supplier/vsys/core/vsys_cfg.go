package vsys_core

/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/base"
	"github.com/virtualeconomy/go-vsys/vsys"
)

var NET = base.Cfg_get("VSYSNetType", "Test")
var	MAINHOST = base.Cfg_get("VSYSMainHOST", "")
var	TESTHOST = base.Cfg_get("VSYSTestHOST", "http://veldidina.vos.systems:9928")
var SEED = base.Cfg_get("VSYSSEED", "")

// =====================================================================

func Vsys_NET() vsys.ChainID {
	if NET == "Main" {
		return vsys.MAIN_NET
	} else {
		return vsys.TEST_NET
	}
}

func Vsys_HOST() string {
	var host string

	if Vsys_NET() == vsys.MAIN_NET {
		host = MAINHOST
	} else {
		host = TESTHOST
	}

	if len(host) == 0 {
		host = TESTHOST
	}

	return host
}

func Vsys_SEED() string {
	return SEED
}