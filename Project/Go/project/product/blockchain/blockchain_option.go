package blockchain

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/auto"
	"dave/public/base"
	"dave/product/blockchain/option"
)

// =====================================================================

func blockchain_option(opt string, param interface{}) (interface{}, int64) {
	if opt == "creat_wallet" {
		return option.Creat_wallet(param)
	} else if opt == "deploy_nft" {
		return option.Deploy_nft(param)
	}

	base.DAVELOG("invalid opt:%v", opt)

	return "", auto.RetCode_invalid_option
}