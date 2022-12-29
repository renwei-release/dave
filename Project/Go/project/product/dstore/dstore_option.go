package dstore

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/base"
	"dave/public/auto"
	"dave/product/dstore/option"
)

// =====================================================================

func dstore_option(opt string, param interface{}) (interface{}, int64) {
	if opt == "add_bin" {
		return option.Add_bin(param)
	}

	base.DAVELOG("invalid opt:%v", opt)

	return "", auto.RetCode_invalid_option
}
