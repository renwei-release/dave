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
	"dave/product/blockchain/supplier/vsys/app/nft"
	"github.com/mitchellh/mapstructure"
)

type DeployNFTReq struct {
	Wallet_address string `json:"wallet_address"`
	User_name string `json:"user_name"`
	Image_url string `json:"image_url"`
	Ipfs_url string `json:"ipfs_url"`
}

type DeployNFTRsp struct {
	Contract_address string `json:"contract_address"`
	TokenID string `json:"tokenid"`
}

func _eth_deploy_nft(req DeployNFTReq) (interface{}, int64) {
	eth_ret, eth_contract_address := eth_nft.Eth_deploy_nft(req.Ipfs_url, req.Ipfs_url)
	vsys_ret, vsys_tokenid := vsys_nft.Vsys_deploy_nft(req.Image_url, req.Image_url)
	if (eth_ret == false) || (vsys_ret == false) {
		base.DAVELOG("Wallet_address:%v User_name:%v URL:%v/%v failed!",
			req.Wallet_address, req.User_name, req.Image_url, req,req.Ipfs_url)
		return "", auto.RetCode_Invalid_call
	}

	rsp := DeployNFTRsp { 
        Contract_address: eth_contract_address, 
        TokenID: vsys_tokenid,
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