package eth_core

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/base"
	"github.com/ethereum/go-ethereum/accounts/keystore"
	"github.com/ethereum/go-ethereum/crypto"
	"crypto/ecdsa"
	"github.com/google/uuid"
)

/*
 * function come from keystore.newKeyFromECDSA
 */
func newKeyFromECDSA(privateKeyECDSA *ecdsa.PrivateKey) *keystore.Key {
	id, err := uuid.NewRandom()
	if err != nil {
		base.DAVELOG("Could not create random uuid: %v", err)
		return nil
	}
	key := &keystore.Key{
		Id:         id,
		Address:    crypto.PubkeyToAddress(privateKeyECDSA.PublicKey),
		PrivateKey: privateKeyECDSA,
	}
	return key
}

func _eth_private_key() string {
	return base.Cfg_get("ETHPrivateKey", "5f2253a32ccb2aa1c419e47213274276286829f9688f78a84c62f24eafc18494")
}

func _eth_keyjson() string {
	return base.Cfg_get("ETHKeyJson", "{\"address\":\"54ccd0456f96a43aa5dce7833bf734e9442955fb\",\"crypto\":{\"cipher\":\"aes-128-ctr\",\"ciphertext\":\"b62c10f3198d7339f7e8a3e2ca72484ff14197328913b03cc9f14c51c7cff830\",\"cipherparams\":{\"iv\":\"67a44bd6acd7046bc4df2fa4c445a551\"},\"kdf\":\"scrypt\",\"kdfparams\":{\"dklen\":32,\"n\":262144,\"p\":1,\"r\":8,\"salt\":\"af095ddb875d8dc5832ee79eb716c5ba77f55ccd7f20963d5768cbf8d16d6c8b\"},\"mac\":\"73ab8ca13d7e9f51176efcfd0899c4c1b1eca869dd4c9d640ccd126f8497d587\"},\"id\":\"f1139069-6552-4fc6-9b4a-823e5e05a938\",\"version\":3}")
}

func _eth_passphrase() string {
	return base.Cfg_get("ETHPassphrase", "123456")
}

func _eth_keyjson_passphrase() (string, string) {
	keyjson := _eth_keyjson()
	passphrase := _eth_passphrase()

	return keyjson, passphrase
}

func _eth_decrypt_keyjson(keyjson string, passphrase string) *keystore.Key {
	userKey, err := keystore.DecryptKey([]byte(keyjson), passphrase)
	if err != nil {
		base.DAVELOG("decrypt key err:%v", err)
		return nil
	}

	return userKey
}

func _eth_keyjson_to_user_key() *keystore.Key {
	keyjson, passphrase := _eth_keyjson_passphrase()
	if keyjson == "" || passphrase == "" {
		base.DAVELOG("empty key:%s or passphrase:%s", keyjson, passphrase)
		return nil
	}

	return _eth_decrypt_keyjson(keyjson, passphrase)
}

func _eth_private_key_to_user_key() *keystore.Key {
	privatekey := _eth_private_key()
	if privatekey == "" {
		base.DAVELOG("empty private key!")
		return nil
	}

	ecdsa_PrivateKey, err := crypto.HexToECDSA(privatekey)
	if err != nil {
		base.DAVELOG("privatekey:%v HexToECDSA err:%v", privatekey, err)
		return nil
	}

	return newKeyFromECDSA(ecdsa_PrivateKey)
}

func _eth_user_key() *keystore.Key {
	key_from := ""

	userKey := _eth_private_key_to_user_key()
	if userKey == nil {
		userKey = _eth_keyjson_to_user_key()
		if userKey != nil {
			key_from = "key json"
		}
	} else {
		key_from = "private key"
	}

	if userKey != nil {
		base.DAVELOG("key_from:%v", key_from)

		base.DAVELOG("(After the test is finished, close the printf!!) account address:%x", crypto.PubkeyToAddress(userKey.PrivateKey.PublicKey))
		base.DAVELOG("(After the test is finished, close the printf!!) private key    :%x", crypto.FromECDSA(userKey.PrivateKey))
		base.DAVELOG("(After the test is finished, close the printf!!) public key     :%x", crypto.FromECDSAPub(&(userKey.PrivateKey.PublicKey)))	
	}

	return userKey
}

// =====================================================================

func Eth_user_key() *keystore.Key {
	return _eth_user_key()
}