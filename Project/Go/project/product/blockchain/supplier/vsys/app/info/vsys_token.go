package vsys_info

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/base"
	"github.com/virtualeconomy/go-v-sdk/vsys"
)

// =====================================================================

func VSYS_token_info(tokenid string) bool {
	base.DAVELOG("tokenid:%v", tokenid)

	info, err := vsys.GetTokenInfo(tokenid)
	if err != nil {
		base.DAVELOG("err:%v", err)
		return false
	}

	base.DAVELOG("tokenid:%v info:%v", tokenid, info)

	return true
}