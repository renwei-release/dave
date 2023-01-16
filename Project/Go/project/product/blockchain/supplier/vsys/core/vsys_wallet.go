package vsys_core

/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/base"
	"github.com/virtualeconomy/go-vsys/vsys"
)

// =====================================================================

func VSYS_wallet_address() string {
	wallet, _ := vsys.NewWalletFromSeedStr(VSYS_SEED())
	account, _ := wallet.GetAccount(VSYS_Chain(), 0)

	base.DAVELOG("account PriKey:%v PubKey:%v Addr:%v",
		account.PriKey.B58Str().Str(), account.PubKey.B58Str().Str(), account.Addr.B58Str().Str())

	return account.Addr.B58Str().Str()
}

func VSYS_account() *vsys.Account {
	wallet, _ := vsys.NewWalletFromSeedStr(VSYS_SEED())
	account, _ := wallet.GetAccount(VSYS_Chain(), 0)
	return account
}