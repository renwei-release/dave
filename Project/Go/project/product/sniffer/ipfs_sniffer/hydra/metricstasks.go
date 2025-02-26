package hydra

import (
	"context"
	"time"

	"github.com/ipfs/go-datastore/query"
	"dave/product/sniffer/ipfs_sniffer/metrics"
	"dave/product/sniffer/ipfs_sniffer/periodictasks"
	"go.opencensus.io/stats"
)

func newProviderRecordsTask(hy *Hydra, d time.Duration) periodictasks.PeriodicTask {
	return periodictasks.PeriodicTask{
		Interval: d,
		Run: func(ctx context.Context) error {
			prs, err := hy.SharedDatastore.Query(query.Query{Prefix: "/providers", KeysOnly: true})
			if err != nil {
				return err
			}
			defer prs.Close()

			// TODO: make fast https://github.com/libp2p/go-libp2p-kad-dht/issues/487
			var provRecords int
			for {
				select {
				case r, ok := <-prs.Next():
					if !ok {
						stats.Record(ctx, metrics.ProviderRecords.M(int64(provRecords)))
						return nil
					}
					if r.Error == nil {
						provRecords++
					}
				case <-ctx.Done():
					return nil
				}
			}
		},
	}
}

func newRoutingTableSizeTask(hy *Hydra, d time.Duration) periodictasks.PeriodicTask {
	return periodictasks.PeriodicTask{
		Interval: d,
		Run: func(ctx context.Context) error {
			var rts int
			for i := range hy.Heads {
				rts += hy.Heads[i].RoutingTable().Size()
			}
			stats.Record(ctx, metrics.RoutingTableSize.M(int64(rts)))
			return nil
		},
	}
}

func newUniquePeersTask(hy *Hydra, d time.Duration) periodictasks.PeriodicTask {
	return periodictasks.PeriodicTask{
		Interval: d,
		Run: func(ctx context.Context) error {
			stats.Record(ctx, metrics.UniquePeers.M(int64(hy.GetUniquePeersCount())))
			return nil
		},
	}
}
