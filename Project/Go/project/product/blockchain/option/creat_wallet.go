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
	"dave/components/bdata"

	"dave/product/blockchain/supplier/eth/core"
	"dave/product/blockchain/supplier/vsys/core"
	"github.com/mitchellh/mapstructure"
)

type CreatWalletReq struct {
	User_name string `json:"user_name"`
}

type CreatWalletRsp struct {
	User_name string `json:"user_name"`
	Eth_address string `json:"eth_address"`
	Eth_Passphrase string `json:"eth_passphrase"`
	Eth_Keystore string `json:"eth_keystore"`
	Vsys_address string `json:"vsys_address"`
	Vsys_seed string `json:"vsys_seed"`
}

func _creat_wallet(param interface{}) (interface{}, int64) {
	req := CreatWalletReq{}
	err := mapstructure.Decode(param, &req)
	if err != nil {
		base.DAVELOG("err:%v", err)
		return "", auto.RetCode_Invalid_parameter
	}

	eth_passphrase := "123456"

	eth_address, eth_keystore := eth_core.Eth_new_wallet(eth_passphrase)
	vsys_address, vsys_seed := vsys_core.Vsys_new_wallet(req.User_name)

	rsp := CreatWalletRsp {
		User_name: req.User_name,
		Eth_address: eth_address,
        Eth_Passphrase: eth_passphrase,
		Eth_Keystore: eth_keystore,
		Vsys_address: vsys_address,
		Vsys_seed: vsys_seed,
    }

	bdata.BDATALOG("wallet", "%v", rsp)

	return rsp, auto.RetCode_OK
}

// =====================================================================

func Creat_wallet(param interface{}) (interface{}, int64) {
	return _creat_wallet(param)
}