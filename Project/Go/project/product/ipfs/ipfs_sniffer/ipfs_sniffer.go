package ipfs_sniffer

import (
	"context"
	"flag"
	"fmt"
	"log"
	"math/rand"
	"os"
	"os/signal"
	"strconv"
	"strings"
	"syscall"
	"time"

	"dave/product/ipfs/ipfs_sniffer/hydra"
	"dave/product/ipfs/ipfs_sniffer/idgen"
	"dave/product/ipfs/ipfs_sniffer/utils"
	"github.com/libp2p/go-libp2p-core/crypto"
	"github.com/libp2p/go-libp2p-core/protocol"
	dht "github.com/libp2p/go-libp2p-kad-dht"
	"github.com/multiformats/go-multiaddr"
)

const (
	defaultBucketSize = 20
)

func IPFSSniffer() {
	nheads := flag.Int("nheads", 1, "Specify the number of Hydra heads to create.")
	dbpath := flag.String("db", "", "Datastore directory (for LevelDB store) or postgresql:// connection URI (for PostgreSQL store)")
	pstorePath := flag.String("pstore", "", "Peerstore directory for LevelDB store (defaults to in-memory store)")
	enableRelay := flag.Bool("enable-relay", false, "Enable libp2p circuit relaying for this node (default false).")
	portBegin := flag.Int("port-begin", 0, "If set, begin port allocation here")
	protocolPrefix := flag.String("protocol-prefix", string(dht.DefaultPrefix), "Specify the DHT protocol prefix (default \"/ipfs\")")
	bucketSize := flag.Int("bucket-size", defaultBucketSize, "Specify the bucket size, note that for some protocols this must be a specific value i.e. for \"/ipfs\" it MUST be 20")
	bootstrapConcurrency := flag.Int("bootstrap-conc", 32, "How many concurrent bootstraps to run")
	bootstrapPeers := flag.String("bootstrap-peers", "", "A CSV list of peer addresses to bootstrap from.")
	stagger := flag.Duration("stagger", 0*time.Second, "Duration to stagger nodes starts by")
	name := flag.String("name", "", "A name for the Hydra (for use in metrics)")
	disableProvGC := flag.Bool("disable-prov-gc", false, "Disable provider record garbage collection (default false).")
	disableProviders := flag.Bool("disable-providers", false, "Disable storing and retrieving provider records, note that for some protocols, like \"/ipfs\", it MUST be false (default false).")
	disableValues := flag.Bool("disable-values", false, "Disable storing and retrieving value records, note that for some protocols, like \"/ipfs\", it MUST be false (default false).")
	disablePrefetch := flag.Bool("disable-prefetch", false, "Disables pre-fetching of discovered provider records (default false).")
	disableProvCounts := flag.Bool("disable-prov-counts", false, "Disable counting provider records for metrics reporting (default false).")
	disableDBCreate := flag.Bool("disable-db-create", false, "Don't create table and index in the target database (default false).")
	flag.Parse()

	fmt.Fprintf(os.Stderr, "🐉 Hydra Booster starting up...\n")

	// Allow short keys. Otherwise, we'll refuse connections from the bootsrappers and break the network.
	// TODO: Remove this when we shut those bootstrappers down.
	crypto.MinRsaKeyBits = 1024

	// Seed the random number generator used by Hydra heads to select a bootstrap peer
	rand.Seed(time.Now().UTC().UnixNano())

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	var idGenerator idgen.IdentityGenerator

	opts := hydra.Options{
		Name:              *name,
		DatastorePath:     *dbpath,
		PeerstorePath:     *pstorePath,
		EnableRelay:       *enableRelay,
		ProtocolPrefix:    protocol.ID(*protocolPrefix),
		BucketSize:        *bucketSize,
		GetPort:           utils.PortSelector(*portBegin),
		NHeads:            *nheads,
		BsCon:             *bootstrapConcurrency,
		Stagger:           *stagger,
		IDGenerator:       idGenerator,
		DisableProvGC:     *disableProvGC,
		DisableProviders:  *disableProviders,
		DisableValues:     *disableValues,
		BootstrapPeers:    mustConvertToMultiaddr(*bootstrapPeers),
		DisablePrefetch:   *disablePrefetch,
		DisableProvCounts: *disableProvCounts,
		DisableDBCreate:   *disableDBCreate,
	}

	_, err := hydra.NewHydra(ctx, opts)
	if err != nil {
		log.Fatalln(err)
	}

	termChan := make(chan os.Signal, 1)
	signal.Notify(termChan, os.Interrupt, syscall.SIGTERM)
	<-termChan // Blocks here until either SIGINT or SIGTERM is received.
	fmt.Println("Received interrupt signal, shutting down...")
}

func mustGetEnvInt(key string, def int) int {
	if os.Getenv(key) == "" {
		return def
	}
	val, err := strconv.Atoi(os.Getenv(key))
	if err != nil {
		log.Fatalln(fmt.Errorf("invalid %s env value: %w", key, err))
	}
	return val
}

func mustGetEnvBool(key string, def bool) bool {
	if os.Getenv(key) == "" {
		return def
	}
	val, err := strconv.ParseBool(os.Getenv(key))
	if err != nil {
		log.Fatalln(fmt.Errorf("invalid %s env value: %w", key, err))
	}
	return val
}

func mustConvertToMultiaddr(csv string) []multiaddr.Multiaddr {
	var peers []multiaddr.Multiaddr
	if csv != "" {
		addrs := strings.Split(csv, ",")
		for _, addr := range addrs {
			ma, err := multiaddr.NewMultiaddr(addr)
			if err != nil {
				log.Fatalln(fmt.Errorf("invalid multiaddr %s: %w", addr, err))
			}
			peers = append(peers, ma)
		}
	}
	return peers
}

// =====================================================================

func IPFSSnifferBooting() {
	go IPFSSniffer()
}
