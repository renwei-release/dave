package p2p
/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"bufio"
	"context"
	"dave/public/base"
	"fmt"

	"github.com/libp2p/go-libp2p/core/host"
	"github.com/libp2p/go-libp2p/core/network"
	"github.com/libp2p/go-libp2p/core/peer"
	"github.com/libp2p/go-libp2p/core/peerstore"
	"github.com/libp2p/go-libp2p/core/protocol"
	"github.com/multiformats/go-multiaddr"
	"github.com/libp2p/go-libp2p/p2p/protocol/circuitv2/client"
)

var _peer_package_size int64 = P2P_load_package_size_cfg()
var _peer_protocol_id protocol.ID

func _peer_handle_stream(s network.Stream) {
	// Create a buffer stream for non blocking read and write.
	rw := bufio.NewReadWriter(bufio.NewReader(s), bufio.NewWriter(s))

	if s.Protocol() != _peer_protocol_id {
		base.DAVELOG("Error protocol:%v != %v", s.Protocol(), _peer_protocol_id)
		return
	}

	local_peer_info := s.Conn().LocalMultiaddr().String() + "/p2p/" + s.Conn().LocalPeer().String()
	remote_peer_info := s.Conn().RemoteMultiaddr().String() + "/p2p/" + s.Conn().RemotePeer().String()

	// Print the stream info.
	base.DAVELOG("%v->%v %v", remote_peer_info, local_peer_info, s.Protocol())

	P2P_io_connection(remote_peer_info, rw)
}

func _peer_start_stream(host host.Host, info *peer.AddrInfo, peer_info string, protocol_id protocol.ID) (* bufio.ReadWriter, error) {
	// Start a stream with the destination.
	// Multiaddress of the destination peer is fetched from the peerstore using 'peerId'.
	s, err := host.NewStream(context.Background(), info.ID, protocol_id)
	if err != nil {
		base.DAVELOG("NewStream err:%v on peer_info:%v protocol_id:%v",
			err, peer_info, protocol_id)
		return nil, err
	}

	// Create a buffered stream so that read and writes are non blocking.
	rw := bufio.NewReadWriter(bufio.NewReader(s), bufio.NewWriter(s))

	return rw, nil
}

func _peer_reserve_relay(host host.Host, info *peer.AddrInfo) {
	if err := host.Connect(context.Background(), *info); err != nil {
		base.DAVELOG("Failed:%v to connect info:%v", err, info)
		return
	}
	_, err := client.Reserve(context.Background(), host, *info)
	if err != nil {
		base.DAVELOG("client failed:%v to receive a relay reservation from host:%v", err, host)
		return
	}

	base.DAVELOG("client received a relay reservation from relay:%v", info)
}

func _peer_connect_relay(host host.Host, relay_peer_id string, dist_peer_id string, protocol_id protocol.ID) (* bufio.ReadWriter, error) {
	relayaddr, err := multiaddr.NewMultiaddr("/p2p/" + relay_peer_id + "/p2p-circuit/p2p/" + dist_peer_id)
	if err != nil {
		base.DAVELOG("err:%v", err)
		return nil, err
	}

	useRelayToDistInfo, err := peer.AddrInfoFromP2pAddr(relayaddr)
	if err := host.Connect(context.Background(), *useRelayToDistInfo); err != nil {
		base.DAVELOG("Unexpected error here. Failed to connect unreachable1 and unreachable2: %v", err)
		return nil, err
	}

	base.DAVELOG("Yep, that worked!")

	s, err := host.NewStream(network.WithUseTransient(context.Background(), string(protocol_id)), useRelayToDistInfo.ID, protocol_id)
	if err != nil {
		base.DAVELOG("Whoops, this should have worked...: ", err)
		return nil, err
	}

	rw := bufio.NewReadWriter(bufio.NewReader(s), bufio.NewWriter(s))

	return rw, nil
}

// =====================================================================

func P2P_start_peer(host host.Host, protocol_id protocol.ID) {
	_peer_protocol_id = protocol_id

	host.SetStreamHandler(_peer_protocol_id, _peer_handle_stream)
}

func P2P_connect_peer(host host.Host, peer_info string, protocol_id protocol.ID) (* bufio.ReadWriter, error) {
	// Turn the destination into a multiaddr.
	maddr, err := multiaddr.NewMultiaddr(peer_info)
	if err != nil {
		base.DAVELOG("invalid peer_info:%v err:%v",
			peer_info, err)
		return nil, err
	}

	// Extract the peer ID from the multiaddr.
	info, err := peer.AddrInfoFromP2pAddr(maddr)
	if err != nil {
		base.DAVELOG("invalid maddr:%v from peer_info:%v err:%v",
			maddr, peer_info, err)
		return nil, err
	}

	// Add the destination's peer multiaddress in the peerstore.
	// This will be used during connection and stream creation by libp2p.
	host.Peerstore().AddAddrs(info.ID, info.Addrs, peerstore.PermanentAddrTTL)

	rw, err := _peer_start_stream(host, info, peer_info, protocol_id)
	if err != nil {
		base.DAVELOG("start_stream err:%v on peer_info:%v protocol_id:%v",
			err, peer_info, protocol_id)
		return nil, err
	}

	_peer_reserve_relay(host, info)

	return rw, nil
}

func P2P_connect_relay(host host.Host, peer_info string, protocol_id protocol.ID) (* bufio.ReadWriter, error) {
	peer_list := P2P_io_peer_list()
	if peer_list == nil {
		base.DAVELOG("peer_list is nil")
		return nil, fmt.Errorf("peer_list is nil")
	}

	dist_peer_id := P2P_peer_info_to_peer_id(peer_info)

	for _, peer_info := range peer_list {
		peer_info_id := P2P_peer_info_to_peer_id(peer_info)
		if peer_info_id != dist_peer_id {
			rw, err := _peer_connect_relay(host, peer_info_id, dist_peer_id, protocol_id)
			if err != nil {
				base.DAVELOG("connect_relay err:%v on peer_info:%v protocol_id:%v",
					err, peer_info, protocol_id)
				continue
			} else {
				return rw, nil
			}
		}
	}

	return nil, fmt.Errorf("connect_relay failed")
}

func P2P_send_peer(rw *bufio.ReadWriter, data []byte, flush_flag bool) error {
	// Send data.
	w_len, w_err := rw.Write(data);
	if w_err != nil {
		base.DAVELOG("Write err:%v on data:%v", w_err, data)
		return w_err
	}
	if w_len != len(data) {
		base.DAVELOG("Write len:%v != %v on data:%v", w_len, len(data), data)
		return fmt.Errorf("Write len:%v != %v on data:%v", w_len, len(data), data)
	}
	if flush_flag == true {
		f_err := rw.Flush();
		if f_err != nil {
			base.DAVELOG("Flush err:%v on data:%v", f_err, data)
			return f_err
		}
	}
	return nil
}

func P2P_recv_peer(rw *bufio.ReadWriter) ([]byte, error) {
	// Read data.
	data := make([]byte, _peer_package_size)
	r_len, r_err := rw.Read(data)
	if r_err != nil {
		base.DAVELOG("Read err:%v", r_err)
		return nil, r_err
	}
	return data[:r_len], nil
}