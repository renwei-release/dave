package vsys_nft

/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/product/blockchain/supplier/vsys/core"
	"dave/public/base"
	"github.com/virtualeconomy/go-vsys/vsys"
	"time"
)

// =====================================================================

func VSYS_deploy_nft(nft_data string, nft_name string) (bool, string) {
	account := vsys_core.VSYS_account()

	nc, err := vsys.RegisterNFTCtrt(account, nft_name)
	if err != nil {
		base.DAVELOG("deploy %s/%s register err:%v", nft_data, nft_name, err)
		return false, ""
	}
	time.Sleep(6 * time.Second)

	issue_resp, err := nc.Issue(account, nft_name, nft_data)
	if err != nil {
		base.DAVELOG("deploy %s/%s issue:%v err:%v", nft_data, nft_name, issue_resp, err)
		return false, ""
	}
	time.Sleep(6 * time.Second)

	send_tx, err := nc.Send(account, string(account.Addr.B58Str()), 1, "")
	if err != nil {
		base.DAVELOG("send %s/%s issue err:%v", nft_data, nft_name, err)
		return false, ""		
	}

	tokenid, err := nc.CtrtId.GetTokId(0)
	if err != nil {
		base.DAVELOG("get tokenid %s/%s issue err:%v", nft_data, nft_name, err)
		return false, ""
	}

	base.DAVELOG("issue_resp:%v", issue_resp)
	base.DAVELOG("tokenid:%v send_tx:%v", tokenid.B58Str().Str(), send_tx)

	return true, tokenid.B58Str().Str()
}