package vsys_core

/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/base"
	"dave/product/blockchain/supplier/vsys/store"
	"github.com/virtualeconomy/go-vsys/vsys"
)

// =====================================================================

func Vsys_new_wallet(user_name string) (string, string) {
	wallet, _ := vsys.GenWallet()
	account, _ := wallet.GetAccount(Vsys_Chain(), 0)

	base.DAVELOG("account PriKey:%v PubKey:%v Addr:%v seed:%v",
		account.PriKey.B58Str().Str(),
		account.PubKey.B58Str().Str(),
		account.Addr.B58Str().Str(),
		wallet.Seed.Str.Str())

	address := account.Addr.B58Str().Str()
	seed := wallet.Seed.Str.Str()

	if len(user_name) > 0 {
		vsys_store.Vsys_store_user_add(user_name, address, seed, "")
	}

	return address, seed 
}

func Vsys_new_account() *vsys.Account {
	wallet, _ := vsys.GenWallet()
	account, _ := wallet.GetAccount(Vsys_Chain(), 0)
	return account
}

func Vsys_my_wallet() string {
	wallet, _ := vsys.NewWalletFromSeedStr(Vsys_SEED())
	account, _ := wallet.GetAccount(Vsys_Chain(), 0)

	base.DAVELOG("account PriKey:%v PubKey:%v Addr:%v",
		account.PriKey.B58Str().Str(),
		account.PubKey.B58Str().Str(),
		account.Addr.B58Str().Str())

	return account.Addr.B58Str().Str()
}

func Vsys_my_account() *vsys.Account {
	wallet, _ := vsys.NewWalletFromSeedStr(Vsys_SEED())
	if wallet == nil {
		base.DAVELOG("maybe invalid seed:%v", Vsys_SEED())
		return nil
	}
	account, _ := wallet.GetAccount(Vsys_Chain(), 0)
	return account
}