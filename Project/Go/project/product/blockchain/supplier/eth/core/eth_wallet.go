package eth_core

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"github.com/ethereum/go-ethereum/accounts/keystore"
	"dave/public/base"
	"dave/public/tools"
)

// =====================================================================

func Eth_new_wallet(passphrase string) (string, string) {
	key_dir := base.Cfg_get("EthKeyDir", "/project/ethereum/keystore")

	ks := keystore.NewPlaintextKeyStore(key_dir)

	account, err := ks.NewAccount(passphrase)
	if err != nil {
		base.DAVELOG("err:%v", err)
		return "", ""
	}

	// URL = keystore:///project/ethereum/keystore/UTC--2022-12-28T12-02-07.254710468Z--5e793a3fd7bdc2b26ed2bfeb6286b14686890faa
	keystore_file := account.URL.String()[11:]
	keystore := tools.T_file_read(keystore_file)

	base.DAVELOG("account Address:%v URL:%v keystore:%v",
		account.Address, account.URL, string(keystore))

	return account.Address.Hex(), string(keystore)
}