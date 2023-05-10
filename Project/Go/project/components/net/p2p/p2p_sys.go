package p2p
/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"time"
)	

func _sys_broadcast_peer_info() {
	peer_list := P2P_io_peer_list()
	for _, peer_info := range peer_list {
		peer_id := P2P_peer_info_to_peer_id(peer_info)
		P2P_sys_send_peer_list(peer_id)
	}
}

func _sys_loop_task() {
	ticker := time.NewTicker(60 * time.Second)
	for {
		select {
		case <-ticker.C:
			_sys_broadcast_peer_info()
		}
	}
}

// =====================================================================

func P2P_sys_init() {
	go _sys_loop_task()
}

func P2P_sys_exit() {

}