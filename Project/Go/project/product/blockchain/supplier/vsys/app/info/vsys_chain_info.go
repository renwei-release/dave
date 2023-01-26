package vsys_info

/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/components/net"
	"dave/public/base"
	"fmt"
)

var _chain_info_url = base.Cfg_get("VSYSChainInfoURL", "http://127.0.0.1:5555")
var _chain_info_head = "{\"Content-Type\": \"application/json\"}"

func _info_hello() string {
	get_url := fmt.Sprintf("%s/", _chain_info_url)

	rsp_res, rsp_body, err := net.Get(get_url)

	if rsp_res.StatusCode != 200 {
		base.DAVELOG("request to %s get Status:%v/%v errs:%v",
		_chain_info_url, rsp_res.Status, rsp_res.StatusCode, err)
		return ""
	}

	base.DAVELOG("rsp_body:%s", rsp_body)

	return rsp_body
}

// =====================================================================

func Vsys_info_total() string {
	return _info_hello()
}