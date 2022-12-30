package vsys_core

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/base"
	"github.com/virtualeconomy/go-v-sdk/vsys"

	"strings"
)

func _vsys_client_url() (string, vsys.NetType) {
	client_url := base.Cfg_get("VSYSClientURL", "http://test.v.systems:9922")

	var nettype vsys.NetType

	if strings.Contains(client_url, "test") == true {
		nettype = vsys.Testnet
	} else {
		nettype = vsys.Mainnet
	}

	return client_url, nettype
}

func _vsys_account() string {
	return base.Cfg_get("VSYSAccount", "ATsSvqRGQeTpeQSGt3eAfNphmJwvnGU9dAw")
}

func _vsys_private_key() string {
	return base.Cfg_get("VSYSPrivateKey", "DvwNVbhTdn7XoCZW3W6YhkJrVk8Rq7NopuYRcz13tCzK")
}

// =====================================================================

func VSYS_client_url() (string, vsys.NetType) {
	return _vsys_client_url()
}

func VSYS_account() *vsys.Account {
	_, nettype :=  _vsys_client_url()

	acc := vsys.InitAccount(nettype)
	acc.BuildFromPrivateKey(_vsys_private_key())

	return acc
}