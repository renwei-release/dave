package p2p
/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"bufio"
	"fmt"
	"time"
	"reflect"
	"dave/public/base"
	"github.com/libp2p/go-libp2p/core/protocol"
)

type PeerType int  

const (  
    PeerType_active_seed PeerType = iota
	PeerType_active_auto 
	PeerType_passive
	PeerType_relay
)

type p2p_io struct {
	peer_type PeerType
	peer_info string
	rw * bufio.ReadWriter
}

var _p2p_io_user_read_func P2PReadFunc = nil
var _p2p_io_array = make(map[string]p2p_io)
var _p2p_io_protocol_id protocol.ID = ""

func (peer_type PeerType) String() string {
	switch peer_type {
	case PeerType_active_seed:
		return "active_seed"
	case PeerType_active_auto:
		return "active_auto"
	case PeerType_passive:
		return "passive"
	case PeerType_relay:
		return "relay"
	default:
		return "unknown"
	}
}

func _io_read_loop(peer_id string, rw * bufio.ReadWriter) {
	for {
		data, err := P2P_recv_peer(rw)
		if err != nil {
			base.DAVELOG("peer_id:%v err:%v", peer_id, err)
			_io_del(peer_id)
			break
		}

		user_data := P2P_head_get_user(data)
		if user_data != nil {
			if _p2p_io_user_read_func != nil {
				_p2p_io_user_read_func(peer_id, user_data)
			}
		} else {
			sys_data := P2P_head_get_system(data)
			if sys_data != nil {
				P2P_sys_recv_data(peer_id, sys_data)
			} else {
				base.DAVELOG("from peer_id:%v receive invalid data:%v", peer_id, string(data))
			}
		}
	}
}

func _io_del(peer_id string) {
	base.DAVELOG("io delete peer_id:%v", peer_id)
	delete(_p2p_io_array, peer_id)
}

func _io_inq(peer_id string) * bufio.ReadWriter {
	p2p_io, ok := _p2p_io_array[peer_id]
	if !ok {
		return nil
	}

	return p2p_io.rw
}

func _io_add(peer_type PeerType, peer_info string, rw * bufio.ReadWriter) {
	peer_id := P2P_peer_info_to_peer_id(peer_info)
	old_rw := _io_inq(peer_id)
	if old_rw != nil {
		base.DAVELOG("io add peer_id:%v already exist, deleted it.", peer_id)
		_io_del(peer_id)
	}

	base.DAVELOG("io connect peer_id:%v peer_info:%v", peer_id, peer_info)
	_p2p_io_array[peer_id] = p2p_io{peer_type, peer_info, rw}

	P2P_set_auto_peer_cfg(peer_info)

	go _io_read_loop(peer_id, rw)
}

func _io_connect_to_seed(peer_info string, protocol_id protocol.ID) {
	peer_id := P2P_peer_info_to_peer_id(peer_info)
	if peer_id == P2P_host_peer_id() {
		// do not connect to myself
		return
	}

	if _io_inq(peer_id) == nil {
		rw, err := P2P_connect_peer(P2P_host(), peer_info, protocol_id)
		if err != nil {
			base.DAVELOG("peer_info:%v err:%v", peer_info, err)
			return
		}

		_io_add(PeerType_active_seed, peer_info, rw)

		P2P_sys_send_hello(peer_id)
	}
}

func _io_connect_to_auto(peer_info string, protocol_id protocol.ID) {
	peer_id := P2P_peer_info_to_peer_id(peer_info)
	if peer_id == P2P_host_peer_id() {
		// do not connect to myself
		return
	}

	if _io_inq(peer_id) == nil {
		rw, err := P2P_connect_peer(P2P_host(), peer_info, protocol_id)
		if err != nil {
			rw, err = P2P_connect_relay(P2P_host(), peer_info, protocol_id)
			if err != nil {
				base.DAVELOG("peer_info:%v err:%v", peer_info, err)
				P2P_del_auto_peer_cfg(peer_info)
				return
			}
		}

		_io_add(PeerType_active_auto, peer_info, rw)

		P2P_sys_send_hello(peer_id)
	}
}

func _io_connect_action(protocol_id protocol.ID) {
	peer_array := P2P_get_seed_peer_cfg()
	for _, peer_info := range peer_array {
		_io_connect_to_seed(peer_info, protocol_id)
	}

	auto_array := P2P_get_auto_peer_cfg()
	for _, auto_info := range auto_array {
		_io_connect_to_auto(auto_info, protocol_id)
	}
}

func _io_connect_loop(protocol_id protocol.ID) {
	_io_connect_action(protocol_id)

	ticker := time.NewTicker(120 * time.Second)
	for {
		select {
		case <-ticker.C:
			_io_connect_action(protocol_id)
		}
	}
}

// =====================================================================

func P2P_io_init(protocol_id protocol.ID) {
	_p2p_io_protocol_id = protocol_id

	go _io_connect_loop(_p2p_io_protocol_id)
}

func P2P_io_exit() {
	
}

func P2P_io_connection(peer_info string, rw * bufio.ReadWriter) {
	_io_add(PeerType_passive, peer_info, rw)
}

func P2P_io_action() {
	_io_connect_action(_p2p_io_protocol_id)
}

func P2P_io_write(peer_id string, data []byte, user_flag bool) error {
	rw := _io_inq(peer_id)
	if rw == nil {
		return fmt.Errorf("peer_id:%v not found", peer_id)
	}

	var head []byte

	if user_flag {
		head = P2P_head_set_user()
	} else {
		head = P2P_head_set_system()
	}
	P2P_send_peer(rw, head, false)

	return P2P_send_peer(rw, data, true)
}

func P2P_io_read(read_func P2PReadFunc) {
	// Check read_func modify
	sf1 := reflect.ValueOf(read_func)
	sf2 := reflect.ValueOf(_p2p_io_user_read_func)
	if sf1.Pointer() != sf2.Pointer() {
		base.DAVELOG("read_func modify:%v->%v!", _p2p_io_user_read_func, read_func)
	}

	_p2p_io_user_read_func = read_func
}

func P2P_io_peer_list() []string {
	var peer_list []string
	for _, value := range _p2p_io_array {
		peer_list = append(peer_list, value.peer_info)
	}
	return peer_list
}

func P2P_io_info() string {
	var info_string string
	for _, value := range _p2p_io_array {
		info_string += fmt.Sprintf("Remote Peer:%v|%v|%v\n",
			value.peer_type.String(), P2P_peer_info_to_peer_id(value.peer_info),
			value.peer_info)
	}
	return info_string
}