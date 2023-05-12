package p2p
/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/base"
	"github.com/libp2p/go-libp2p/core/protocol"
	"fmt"
)

type P2PReadFunc func(string, []byte)

func _p2p_local_info() string {
	var info_string string = ""

	local_peer_infos := P2P_host_peer_infos()
	info_string += fmt.Sprintf("Local Peer ID:%s\n", P2P_host_peer_id())
	for _, local_peer_info := range local_peer_infos {
		info_string += fmt.Sprintf("Local Peer:%s\n", local_peer_info)
	}

	return info_string
}

func _p2p_cfg_info() string {
	var info_string string = ""

	peer_list := P2P_get_seed_peer_cfg()
	for _, peer_cfg := range peer_list {
		info_string += fmt.Sprintf("CFG Peer:%s\n", peer_cfg)
	}

	return info_string
}

// =====================================================================

func P2P_init(protocol_id string, tcp_port string, udp_port string) error {
	var err error

	P2P_cfg_init()

	host, err := P2P_host_creat(tcp_port, udp_port)

	if err != nil {
		base.DAVELOG("error:%v", err)
		return fmt.Errorf("P2P_init error:%v", err)
	} else {
		P2P_start_peer(host, protocol.ID(protocol_id))
		P2P_io_init(protocol.ID(protocol_id))
		P2P_sys_init()

		base.DAVEDEBUG("Peer ID:%v", host.ID())
		base.DAVEDEBUG("Listen addr:%v", host.Addrs())
	}

	return nil
}

func P2P_exit() {
	P2P_sys_exit()
	P2P_io_exit()
	P2P_host_close()
	P2P_cfg_exit()
}

func P2P_write(peer_id string, data []byte) error {
	return P2P_io_write(peer_id, data, true)
}

func P2P_read(read_func P2PReadFunc) {
	P2P_io_read(read_func)
}

func P2P_info() string {

	var info_string string = ""

	info_string += _p2p_local_info()
	info_string += _p2p_cfg_info()
	info_string += P2P_io_info()

	return info_string
}