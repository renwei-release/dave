package eth_core

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"errors"
	"github.com/ethereum/go-ethereum/accounts/abi/bind"
)

// =====================================================================

func Eth_transactor() (*bind.TransactOpts, error) {
	userkey := Eth_user_key()
	if userkey == nil {
		return nil, errors.New("empty userKey")
	}

	auth := bind.NewKeyedTransactor(userkey.PrivateKey)

	return auth, nil
}