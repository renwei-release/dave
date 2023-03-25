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
	"dave/product/dstore/supplier/ipfs"
	"encoding/base64"
	"github.com/mitchellh/mapstructure"
)

type AddBinReq struct {
	Bin_data string `json:"bin_data"`
	Bin_name string `json:"bin_name"`
}

type AddBinRsp struct {
	IPFS_URL string `json:"ipfs_url"`
	Bin_name string `json:"bin_name"`
}

func _ipfs_add_bin(bin_data []byte, bin_name string) (string, string) {
	if len(bin_name) == 0 {
		bin_name = "NFT.jpg"
	}

	cid := ipfs.IPFS_add_bin(bin_data, bin_name)

	return "https://ipfs.io/ipfs/"+cid, bin_name
}

func _add_bin(param interface{}) (interface{}, int64) {
	req := AddBinReq{}
	err := mapstructure.Decode(param, &req)
	if err != nil {
		return "", auto.RetCode_Invalid_parameter
	}

	bin_data, err := base64.StdEncoding.DecodeString(req.Bin_data)
	if err != nil {
		base.DAVELOG("err:%v base64_data:%v", err, req.Bin_data)
		return "", auto.RetCode_decode_failed
	}

	ipfs_url, bin_name := _ipfs_add_bin(bin_data, req.Bin_name)
	if len(ipfs_url) == 0 {
		return "", auto.RetCode_store_data_failed
	}

	rsp := AddBinRsp { 
        IPFS_URL: ipfs_url,
		Bin_name: bin_name,
    }

	base.DAVELOG("ipfs_url:%v", ipfs_url)

	return rsp, auto.RetCode_OK
}

// =====================================================================

func Add_bin(param interface{}) (interface{}, int64) {
	return _add_bin(param)
}