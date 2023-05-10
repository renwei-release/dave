package p2p
/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/base"
	"dave/public/tools"
	"github.com/libp2p/go-libp2p/core/crypto"
	"encoding/json"
)

var CFG_P2P_SEED_PEER = "P2PSeedPeer"
var CFG_P2P_AUTO_PEER = "P2PAutoPeer"

func _cfg_peer_info_update(peer_list []string, peer_info string) []string {
	updateArray := []string{}

	peer_info_id := P2P_peer_info_to_peer_id(peer_info)

	for _, peer_cfg := range peer_list {
		if peer_cfg == "" {
			continue
		}

		peer_cfg_id := P2P_peer_info_to_peer_id(peer_cfg)
		if peer_info_id == peer_cfg_id {
			updateArray = append(updateArray, peer_info)
		} else {
			updateArray = append(updateArray, peer_cfg)
		}
	}

	return updateArray
}

func _cfg_peer_info_deleted(peer_list []string, peer_info string) []string {
	updateArray := []string{}

	for _, peer_cfg := range peer_list {
		if peer_cfg == "" {
			continue
		}

		if peer_cfg != peer_info {
			updateArray = append(updateArray, peer_cfg)
		}
	}

	return updateArray
}

func _cfg_peer_info_duplicates(peer_info []string) []string {
	strArray := tools.T_stdio_remove_string_duplicates(peer_info)

	keys := make(map[string]bool)
	uniqueArray := []string{}

	for _, peer_info := range strArray {
		if peer_info == "" {
			continue
		}

		peer_id := P2P_peer_info_to_peer_id(peer_info)

		if _, value := keys[peer_id]; !value {
			keys[peer_id] = true
			uniqueArray = append(uniqueArray, peer_info)
		}
	}

	return uniqueArray
}

func _cfg_get_peer_cfg(json_string string) []string {
	if json_string == "" {
		return nil
	}
	var strArray []string  
	err := json.Unmarshal([]byte(json_string), &strArray)  
	if err != nil {  
		base.DAVELOG("error:%v", err)
		return nil
	}
	return _cfg_peer_info_duplicates(strArray)
}

func _cfg_set_peer_cfg(json_string string, peer_info string) ([]byte, error) {
	if json_string == "" {
		json_string = "[]"
	}
	var strArray []string  
	err := json.Unmarshal([]byte(json_string), &strArray)  
	if err != nil {  
		base.DAVELOG("error:%v", err)
		return nil, err
	}
	updateArray := _cfg_peer_info_update(strArray, peer_info)

	uniqueStrArray := _cfg_peer_info_duplicates(append(updateArray, peer_info))

	jsonBytes, err := json.Marshal(uniqueStrArray)
	if err != nil {
		base.DAVELOG("error:%v", err)
		return nil, err
	}

	return jsonBytes, nil
}

func _cfg_del_peer_cfg(json_string string, peer_info string) ([]byte, error) {
	if json_string == "" {
		json_string = "[]"
	}
	var strArray []string  
	err := json.Unmarshal([]byte(json_string), &strArray)  
	if err != nil {  
		base.DAVELOG("error:%v", err)
		return nil, err
	}
	updateArray := _cfg_peer_info_deleted(strArray, peer_info)

	jsonBytes, err := json.Marshal(updateArray)
	if err != nil {
		base.DAVELOG("error:%v", err)
		return nil, err
	}

	return jsonBytes, nil
}

// =====================================================================

func P2P_load_package_size_cfg() int64 {
	return base.Cfg_get_ub("P2PPackageSize", 4096)
}

func P2P_load_private_key_cfg() crypto.PrivKey {
	priv, _ := P2P_key()
	if priv == nil {
		base.DAVELOG("invalid private key!")
		return nil
	}
	return priv
}

func P2P_get_seed_peer_cfg() []string {
	json_string := base.Cfg_get(CFG_P2P_SEED_PEER, "")

	return _cfg_get_peer_cfg(json_string)
}

func P2P_set_seed_peer_cfg(peer_info string) error {
	json_string := base.Cfg_get(CFG_P2P_SEED_PEER, "")

	jsonBytes, err := _cfg_set_peer_cfg(json_string, peer_info)
	if err != nil {
		return err
	}

	base.Cfg_set(CFG_P2P_SEED_PEER, string(jsonBytes))

	return nil
}

func P2P_get_auto_peer_cfg() []string {
	json_string := base.Cfg_get(CFG_P2P_AUTO_PEER, "")

	return _cfg_get_peer_cfg(json_string)
}

func P2P_set_auto_peer_cfg(peer_info string) error {
	json_string := base.Cfg_get(CFG_P2P_AUTO_PEER, "")

	jsonBytes, err := _cfg_set_peer_cfg(json_string, peer_info)
	if err != nil {
		return err
	}

	base.Cfg_set(CFG_P2P_AUTO_PEER, string(jsonBytes))

	return nil
}

func P2P_del_auto_peer_cfg(peer_info string) error {
	json_string := base.Cfg_get(CFG_P2P_AUTO_PEER, "")

	jsonBytes, err := _cfg_del_peer_cfg(json_string, peer_info)
	if err != nil {
		return err
	}

	base.Cfg_set(CFG_P2P_AUTO_PEER, string(jsonBytes))

	return nil
}