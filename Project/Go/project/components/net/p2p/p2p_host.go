package p2p
/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/base"
	"fmt"
	"strconv"
	"os/exec"
	"github.com/libp2p/go-libp2p"
	"github.com/libp2p/go-libp2p/core/host"
	"github.com/libp2p/go-libp2p/core/crypto"
	"github.com/multiformats/go-multiaddr"
	"github.com/libp2p/go-libp2p/core/peer"
)

var _global_host host.Host = nil

func _p2p_host_load_port(tcp_port_string string, udp_port_string string) (int, int, error) {
	tcp_port, err := strconv.Atoi(tcp_port_string)
    if err != nil {
		 return 0, 0, err
    }
	udp_port, err := strconv.Atoi(udp_port_string)
    if err != nil {
		 return 0, 0, err
    }
	if ((tcp_port <= 0) && (udp_port <= 0)) || tcp_port > 65535 || udp_port > 65535 {
		return 0, 0, fmt.Errorf("invalid tcp_port:%v udp_port:%v", tcp_port, udp_port)
	}

	return tcp_port, udp_port, err
}

func _p2p_host_creat_by_default() (host.Host, error) {
	return libp2p.New()
}

func _p2p_host_creat_by_priv(priv crypto.PrivKey) (host.Host, error) {
	return libp2p.New(libp2p.Identity(priv))
}

func _p2p_host_creat_by_port(priv crypto.PrivKey, tcp_port_string string, udp_port_string string) (host.Host, error) {
	tcp_port, udp_port, err := _p2p_host_load_port(tcp_port_string, udp_port_string)
	if err != nil {
		return nil, err
	}

	ip4_tcp := fmt.Sprintf("/ip4/0.0.0.0/tcp/%v", tcp_port)
	ip6_tcp := fmt.Sprintf("/ip6/::/tcp/%v", tcp_port)
	ip4_udp := fmt.Sprintf("/ip4/0.0.0.0/udp/%v/quic", udp_port)
	ip6_udp := fmt.Sprintf("/ip6/::/udp/%v/quic", udp_port)

	// start a libp2p node with default settings
	host, err := libp2p.New(
		libp2p.Identity(priv),
		libp2p.ListenAddrStrings(ip4_tcp, ip6_tcp, ip4_udp, ip6_udp))
	if err != nil {
		base.DAVELOG("error:%v", err)
		return host, err
	}

	return host, nil
}

func _p2p_host_for_quic() {
	// https://github.com/quic-go/quic-go/wiki/UDP-Receive-Buffer-Size
    cmd := exec.Command("/sbin/sysctl", "-w", "net.core.rmem_max=2500000")  
    output, err := cmd.Output()  

    if err != nil {  
        base.DAVELOG("error:%v", err)
        return  
    }  
  
    base.DAVELOG("%v", string(output))
}

// =====================================================================

func P2P_host_creat(tcp_port string, udp_port string) (host.Host, error) {
	priv := P2P_load_private_key_cfg()
	if priv == nil {
		base.DAVELOG("invalid private key!")
		return nil, nil
	}

	var host = _global_host
	var err error

	_p2p_host_for_quic()

	if priv == nil {
		host, err = _p2p_host_creat_by_default()
	} else {
		host, err = _p2p_host_creat_by_port(priv, tcp_port, udp_port)
		if err != nil {
			base.DAVELOG("now creat host by private key!")
			host, err = _p2p_host_creat_by_priv(priv)
		}
	}

	if _global_host == nil {
		_global_host = host
	}

	P2P_relay_start(_global_host)

	return host, nil
}

func P2P_host_close() {
	P2P_relay_close()

	if _global_host != nil {
		_global_host.Close()
		_global_host = nil
	}
}

func P2P_host() host.Host {
	return _global_host
}

func P2P_host_peer_id() string {
	return _global_host.ID().Pretty()
}

func P2P_host_peer_infos() []multiaddr.Multiaddr {
	peerInfo := peer.AddrInfo{
		ID:    _global_host.ID(),
		Addrs: _global_host.Addrs(),
	}
	infos, err := peer.AddrInfoToP2pAddrs(&peerInfo)
	if err != nil {
		base.DAVELOG("error:%v", err)
		return nil
	} else {
		return infos
	}
}