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
	User_name string `json:"user_name"`
	Image_url string `json:"image_url"`
	Ipfs_url string `json:"ipfs_url"`
}

type DeployNFTRsp struct {
	User_name string `json:"user_name"`
	Image_url string `json:"image_url"`
	Ipfs_url string `json:"ipfs_url"`
	Eth_contract_address string `json:"eth_contract_address"`
	Vsys_tokenID string `json:"vsys_tokenid"`
}

func _eth_deploy_nft(req DeployNFTReq) (interface{}, int64) {
	_, eth_contract_address := eth_nft.Eth_deploy_nft(req.Ipfs_url, req.Ipfs_url)
	vsys_ret, vsys_tokenid := vsys_nft.Vsys_deploy_nft(req.User_name, req.Image_url, req.Ipfs_url)
	if vsys_ret == false {
		base.DAVELOG("user_name:%v URL:%v/%v failed!",
			req.User_name, req.Image_url, req,req.Ipfs_url)
		return "", auto.RetCode_Invalid_call
	}

	rsp := DeployNFTRsp { 
		User_name: req.User_name,
		Image_url: req.Image_url,
		Ipfs_url: req.Ipfs_url,
        Eth_contract_address: eth_contract_address, 
        Vsys_tokenID: vsys_tokenid,
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