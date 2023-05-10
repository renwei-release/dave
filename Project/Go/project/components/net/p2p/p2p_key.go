package p2p
/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/base"
	"github.com/libp2p/go-libp2p/core/crypto"
)

var CFG_P2P_PRIVATE_KEY = "P2PPrivateKey"
var CFG_P2P_PUBLIC_KEY = "P2PPublicKey"

func _p2p_generate_key() (string, string) {
	priv, pub, err := crypto.GenerateKeyPair(crypto.RSA, 2048)
	if err != nil {
		base.DAVELOG("error:%v", err)
		return "", ""
	}

	priv_bytes, err := crypto.MarshalPrivateKey(priv)
	if err != nil {
		base.DAVELOG("error:%v", err)
		return "", ""
	}
	pub_bytes, err := crypto.MarshalPublicKey(pub)
	if err != nil {
		base.DAVELOG("error:%v", err)
		return "", ""
	}

	return crypto.ConfigEncodeKey(priv_bytes), crypto.ConfigEncodeKey(pub_bytes)
}

func _p2p_load_key() (string, string) {
	private_key_string := base.Cfg_get(CFG_P2P_PRIVATE_KEY, "")
	public_key_string := base.Cfg_get(CFG_P2P_PUBLIC_KEY, "")

	if private_key_string == "" || public_key_string == "" {
		private_key_string, public_key_string = _p2p_generate_key()

		base.Cfg_set(CFG_P2P_PRIVATE_KEY, private_key_string)
		base.Cfg_set(CFG_P2P_PUBLIC_KEY, public_key_string)
	}

	return private_key_string, public_key_string
}

// =====================================================================

func P2P_key() (crypto.PrivKey, crypto.PubKey) {
	private_key_string, public_key_string := _p2p_load_key()

	priv_bytes, err := crypto.ConfigDecodeKey(private_key_string)
	if err != nil {
		base.DAVELOG("error:%v", err)
		return nil, nil
	}
	pub_bytes, err := crypto.ConfigDecodeKey(public_key_string)
	if err != nil {
		base.DAVELOG("error:%v", err)
		return nil, nil
	}

	priv, err := crypto.UnmarshalPrivateKey(priv_bytes)
	if err != nil {
		base.DAVELOG("error:%v", err)
		return nil, nil
	}
	pub, err := crypto.UnmarshalPublicKey(pub_bytes)
	if err != nil {
		base.DAVELOG("error:%v", err)
		return nil, nil
	}

	return priv, pub
}