package option

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/base"
	"dave/public/auto"
	"dave/product/dstore/ipfs"
	"encoding/base64"
	"github.com/mitchellh/mapstructure"
)

type AddBinReq struct {
	Bin_data string `json:"bin_data"`
}

type AddBinRsp struct {
	URL string `json:"url"`
}

func _ipfs_add_bin(base64_data string) string {
	bin_data, err := base64.StdEncoding.DecodeString(base64_data)
	if err != nil {
		base.DAVELOG("err:%v", err)
		return ""
	}

	cid := ipfs.IPFS_add_bin(bin_data)

	return "https://ipfs.io/ipfs/"+cid
}

func _add_bin(param interface{}) (interface{}, int64) {
	req := AddBinReq{}
	err := mapstructure.Decode(param, &req)
	if err != nil {
		return "", auto.RetCode_Invalid_parameter
	}

	url := _ipfs_add_bin(req.Bin_data)
	if len(url) == 0 {
		return "", auto.RetCode_store_data_failed
	}

	rsp := AddBinRsp { 
        URL: url,
    }

	return rsp, auto.RetCode_OK
}

// =====================================================================

func Add_bin(param interface{}) (interface{}, int64) {
	return _add_bin(param)
}