package eth

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"github.com/ethereum/go-ethereum/accounts/abi/bind"
	"strings"
	"errors"
	"dave/public/base"
)

func _eth_key() string {
	return base.Cfg_get("ETHKey", "{\"address\":\"54ccd0456f96a43aa5dce7833bf734e9442955fb\",\"crypto\":{\"cipher\":\"aes-128-ctr\",\"ciphertext\":\"b62c10f3198d7339f7e8a3e2ca72484ff14197328913b03cc9f14c51c7cff830\",\"cipherparams\":{\"iv\":\"67a44bd6acd7046bc4df2fa4c445a551\"},\"kdf\":\"scrypt\",\"kdfparams\":{\"dklen\":32,\"n\":262144,\"p\":1,\"r\":8,\"salt\":\"af095ddb875d8dc5832ee79eb716c5ba77f55ccd7f20963d5768cbf8d16d6c8b\"},\"mac\":\"73ab8ca13d7e9f51176efcfd0899c4c1b1eca869dd4c9d640ccd126f8497d587\"},\"id\":\"f1139069-6552-4fc6-9b4a-823e5e05a938\",\"version\":3}")
}

func _eth_passphrase() string {
	return base.Cfg_get("ETHPassphrase", "123456")
}

func _eth_key_passphrase() (string, string) {
	key := _eth_key()
	passphrase := _eth_passphrase()

	return key, passphrase
}

// =====================================================================

func Eth_transactor() (*bind.TransactOpts, error) {
	key, passphrase := _eth_key_passphrase()
	if key == "" || passphrase == "" {
		base.DAVELOG("empty key:%s or passphrase:%s", key, passphrase)
		return nil, errors.New("empty key or passphrase")
	}

	auth, err := bind.NewTransactor(strings.NewReader(key), passphrase)
	if nil != err {
		base.DAVELOG("Failed to create authorized transactor: %v", err)
		return nil, err
	}

	return auth, nil
}