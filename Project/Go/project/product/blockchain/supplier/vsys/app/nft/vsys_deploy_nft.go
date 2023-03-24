package vsys_nft

/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/components/bdata"
	"dave/product/blockchain/supplier/vsys/core"
	"dave/product/blockchain/supplier/vsys/store"
	"dave/public/base"
	"github.com/virtualeconomy/go-vsys/vsys"
	"time"
)

func _vsys_deploy_nft(user_name string, image_url string, ipfs_url string) (bool, string) {
	account := vsys_core.Vsys_my_account()

	nc, err := vsys.RegisterNFTCtrt(account, image_url)
	if err != nil {
		base.DAVELOG("deploy %s/%s register err:%v", image_url, ipfs_url, err)
		return false, ""
	}
	time.Sleep(6 * time.Second)

	issue_resp, err := nc.Issue(account, image_url, ipfs_url)
	if err != nil {
		base.DAVELOG("deploy %s/%s issue:%v err:%v", image_url, ipfs_url, issue_resp, err)
		return false, ""
	}
	time.Sleep(6 * time.Second)

	send_tx, err := nc.Send(account, string(account.Addr.B58Str()), 1, "")
	if err != nil {
		base.DAVELOG("send %s/%s issue err:%v", image_url, ipfs_url, err)
		return false, ""		
	}

	tokenid, err := nc.CtrtId.GetTokId(0)
	if err != nil {
		base.DAVELOG("get tokenid %s/%s issue err:%v", image_url, ipfs_url, err)
		return false, ""
	}

	tokenid_str := tokenid.B58Str().Str()

	bdata.BDATALOG("NFT", "user_name:%v tokenid:%v image_url:%v ipfs_url:%v issue_resp:%v send_tx:%v",
		user_name, tokenid_str, image_url, ipfs_url,
		issue_resp, send_tx)

	vsys_store.Vsys_store_nft_add(tokenid_str, user_name, image_url, ipfs_url)

	return true, tokenid_str
}

// =====================================================================

func Vsys_deploy_nft(user_name string, image_url string, ipfs_url string) (bool, string) {
	_, _, tokenid := vsys_store.Vsys_store_user_inq(user_name)

	if tokenid == "" {
		return _vsys_deploy_nft(user_name, image_url, ipfs_url)
	} else {
		base.DAVELOG("The user:%v has been deploy NFT:%v", user_name, tokenid)
		return true, tokenid
	}
}