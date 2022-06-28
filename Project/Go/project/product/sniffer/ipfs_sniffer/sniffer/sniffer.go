package sniffer

import (
	"context"
	"dave/product/sniffer/ipfs_sniffer/sniffer/proxy"
	"fmt"
	"github.com/ipfs/go-cid"
	"github.com/ipfs/go-datastore"
	"github.com/libp2p/go-libp2p-core/peer"
	"github.com/multiformats/go-base32"
	"log"
	"strings"
)

var errInvalidKeyNamespaces = fmt.Errorf("not enough namespaces in provider record key")

func keyToCID(k datastore.Key) (cid.Cid, error) {
	nss := k.Namespaces()
	if len(nss) < 2 {
		return cid.Undef, errInvalidKeyNamespaces
	}

	b, err := base32.RawStdEncoding.DecodeString(nss[1])
	if err != nil {
		return cid.Undef, err
	}

	_, c, err := cid.CidFromBytes(b)
	if err != nil {
		return cid.Undef, err
	}

	return c, nil
}

// Source: https://github.com/libp2p/go-libp2p-kad-dht/blob/9304f5575ea4c578d1316c2cf695a06b65c88dbe/providers/providers_manager.go#L339
func keyToPeerID(k datastore.Key) (peer.ID, error) {
	kStr := k.String()

	lix := strings.LastIndex(kStr, "/")

	decstr, err := base32.RawStdEncoding.DecodeString(kStr[lix+1:])
	if err != nil {
		return "", err
	}

	return peer.ID(decstr), nil
}

func AfterPut(k datastore.Key, v []byte, err error) error {

	cid, err := keyToCID(k)
	if err != nil {

		return nil
	}

	pid, err := keyToPeerID(k)

	log.Printf("[NEW] cid[%s] pid[%s] \n", cid, pid)

	return err
}

// =====================================================================

func SnifferStart(ctx context.Context, ds datastore.Batching) (context.Context, datastore.Batching, error) {
	ds = proxy.New(ds, AfterPut)
	return ctx, ds, nil
}
