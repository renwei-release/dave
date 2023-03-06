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

	"dave/product/blockchain/supplier/eth/core"
	"dave/product/blockchain/supplier/vsys/core"
	"github.com/mitchellh/mapstructure"
)

type CreatWalletReq struct {
	Passphrase string `json:"passphrase"`
}

type CreatWalletRsp struct {
	Address string `json:"address"`
	Eth_address string `json:"eth_address"`
	Passphrase string `json:"passphrase"`
	Keystore string `json:"keystore"`
	Vsys_address string `json:"vsys_address"`
	Vsys_seed string `json:"vsys_seed"`
}

func _eth_creat_wallet(req CreatWalletReq) (interface{}, int64) {
	eth_address, keystore := eth_core.Eth_new_wallet(req.Passphrase)
	vsys_address, vsys_seed := vsys_core.Vsys_new_wallet()

	rsp := CreatWalletRsp { 
        Address: eth_address,
		Eth_address: eth_address,
        Passphrase: req.Passphrase,
		Keystore: keystore,
		Vsys_address: vsys_address,
		Vsys_seed: vsys_seed,
    }

	return rsp, auto.RetCode_OK
}

func _creat_wallet(param interface{}) (interface{}, int64) {
	req := CreatWalletReq{}
	err := mapstructure.Decode(param, &req)
	if err != nil {
		base.DAVELOG("err:%v", err)
		return "", auto.RetCode_Invalid_parameter
	}

	return _eth_creat_wallet(req)
}

// =====================================================================

func Creat_wallet(param interface{}) (interface{}, int64) {
	return _creat_wallet(param)
}