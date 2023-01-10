package option

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/auto"
	"dave/public/base"

	"dave/product/blockchain/supplier/eth/app/nft"
	"github.com/mitchellh/mapstructure"
)

type DeployNFTReq struct {
	Wallet_address string `json:"wallet_address"`
	Passphrase string `json:"passphrase"`
	URL string `json:"url"`
}

type DeployNFTRsp struct {
	Contract_address string `json:"contract_address"`
	TokenID string `json:"tokenid"`
}

func _eth_deploy_nft(req DeployNFTReq) (interface{}, int64) {
	ret, wallet_address := eth_nft.Eth_deploy_nft(req.URL, req.URL)
	if ret == false {
		base.DAVELOG("Wallet_address:%v Passphrase:%v URL:%v failed!",
			req.Wallet_address, req.Passphrase, req.URL)
		return "", auto.RetCode_Invalid_call
	}

	rsp := DeployNFTRsp { 
        Contract_address: wallet_address, 
        TokenID: "0",
    }

	return rsp, auto.RetCode_OK
}

func _deploy_nft(param interface{}) (interface{}, int64) {
	req := DeployNFTReq{}
	err := mapstructure.Decode(param, &req)
	if err != nil {
		return "", auto.RetCode_Invalid_parameter
	}

	return _eth_deploy_nft(req)
}

// =====================================================================

func Deploy_nft(param interface{}) (interface{}, int64) {
	return _deploy_nft(param)
}