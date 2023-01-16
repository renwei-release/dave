package pg_core

/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/components/net"
	"dave/public/base"
	"encoding/json"
	"fmt"
)

var _json_rpc_url = base.Cfg_get("PolygonJsonRPCURL", "http://127.0.0.1:30002")
var _json_api_head = "{\"Content-Type\": \"application/json\"}"

// =====================================================================

func PG_chainId() string {
	body := fmt.Sprintf("{\"jsonrpc\":\"2.0\",\"method\":\"eth_chainId\",\"params\":[],\"id\":%d}", 1)

	rsp_res, rsp_body, err := net.Post(_json_rpc_url, _json_api_head, body)

	if rsp_res.StatusCode != 200 {
		base.DAVELOG("request to %s get Status:%v/%v errs:%v",
			_json_rpc_url, rsp_res.Status, rsp_res.StatusCode, err)
		return ""
	}

	m := make(map[string]string)
	json.Unmarshal([]byte(rsp_body), &m)

	return m["result"]
}