package p2p
/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"os"
	"fmt"
	"dave/public/base"
	"encoding/json"
)

type PeerSysPkgType int32  

const (  
    PeerSysPkgType_say_hello PeerSysPkgType = iota
	PeerSysPkgType_answer_hello
	PeerSysPkgType_peer_list
)

func _sys_send_say_hello(peer_id string) {
	host_name, _ := os.Hostname()
	hello_msg := fmt.Sprintf("hello, I am %v/%v", base.Dave_verno(), host_name)
	_sys_send_data(peer_id, PeerSysPkgType_say_hello, []byte(hello_msg))
}

func _sys_recv_say_hello(peer_id string, data []byte) {
	base.DAVELOG("peer_id:%v version:%v", peer_id, string(data))
	_sys_send_answer_hello(peer_id)
}

func _sys_send_answer_hello(peer_id string) {
	host_name, _ := os.Hostname()
	hello_msg := fmt.Sprintf("hello, I am %v/%v", base.Dave_verno(), host_name)
	_sys_send_data(peer_id, PeerSysPkgType_answer_hello, []byte(hello_msg))
}

func _sys_recv_answer_hello(peer_id string, data []byte) {
	base.DAVELOG("peer_id:%v version:%v", peer_id, string(data))
}

func _sys_send_peer_list(peer_id string, peer_list []string) {
	jsonArray, _ := json.Marshal(peer_list)
	_sys_send_data(peer_id, PeerSysPkgType_peer_list, jsonArray)
}

func _sys_recv_peer_list(peer_id string, data []byte) {
	var peer_list []string
	err := json.Unmarshal(data, &peer_list)
	if err != nil {
		base.DAVELOG("error:%v", err)
		return
	}

	for _, peer_info := range peer_list {
		base.DAVELOG("system recv peer_info:%v", peer_info)
		P2P_set_auto_peer_cfg(peer_info)
	}
}

func _sys_send_data(peer_id string, pck_type PeerSysPkgType, data []byte) {
    var type_buf [4]byte

	type_buf[3] = uint8(pck_type)
	type_buf[2] = uint8(pck_type >> 8)
	type_buf[1] = uint8(pck_type >> 16)
	type_buf[0] = uint8(pck_type >> 24)

	data = append(type_buf[:], data...)

	P2P_io_write(peer_id, data, false)
}

func _sys_recv_data(peer_id string, pck_type PeerSysPkgType, data []byte) {

	switch pck_type {
		case PeerSysPkgType_say_hello:
			_sys_recv_say_hello(peer_id, data)
		case PeerSysPkgType_answer_hello:
			_sys_recv_answer_hello(peer_id, data)
		case PeerSysPkgType_peer_list:
			_sys_recv_peer_list(peer_id, data)
		default:
			base.DAVELOG("invalid pck_type:%v", pck_type)
	}
}

// =====================================================================

func P2P_sys_recv_data(peer_id string, data []byte) {
	if len(data) < 4 {
		base.DAVELOG("invalid data len:%v/%v", len(data), data)
		return
	}

	var pck_type PeerSysPkgType
	pck_type = PeerSysPkgType(data[0]) << 24 | PeerSysPkgType(data[1]) << 16 | PeerSysPkgType(data[2]) << 8 | PeerSysPkgType(data[3])

	_sys_recv_data(peer_id, pck_type, data[4:])
}

func P2P_sys_send_hello(peer_id string) {
	_sys_send_say_hello(peer_id)
}

func P2P_sys_send_peer_list(peer_id string) {
	peer_list := P2P_io_peer_list()

	_sys_send_peer_list(peer_id, peer_list)
}