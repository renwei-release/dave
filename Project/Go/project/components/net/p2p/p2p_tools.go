package p2p
/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/base"

	"github.com/libp2p/go-libp2p/core/peer"
	"github.com/multiformats/go-multiaddr"
)

// =====================================================================

func P2P_peer_info_to_peer_id(peer_info string) string {
	maddr, err := multiaddr.NewMultiaddr(peer_info)
	if err != nil {
		base.DAVELOG("invalid peer_info:%v err:%v", peer_info, err)
		return ""
	}

	// Extract the peer ID from the multiaddr.
	info, err := peer.AddrInfoFromP2pAddr(maddr)
	if err != nil {
		base.DAVELOG("invalid maddr:%v from peer_info:%v err:%v", maddr, peer_info, err)
		return ""
	}

	return info.ID.String()
}