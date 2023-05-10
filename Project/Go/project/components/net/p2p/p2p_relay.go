package p2p
/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

// https://blog.csdn.net/Cake_C/article/details/127630718

import (  
	"dave/public/base"

	"github.com/libp2p/go-libp2p/core/host"
	"github.com/libp2p/go-libp2p/p2p/protocol/circuitv2/relay"
)

var _global_relay * relay.Relay = nil

// =====================================================================

func P2P_relay_start(host host.Host) error {
	relay, err := relay.New(host)
	if err != nil {
		base.DAVELOG("error:%v", err)
		return err
	}

	if _global_relay == nil {
		_global_relay = relay
	}

	return nil
}

func P2P_relay_close() {
	if _global_relay != nil {
		_global_relay.Close()
		_global_relay = nil
	}
}