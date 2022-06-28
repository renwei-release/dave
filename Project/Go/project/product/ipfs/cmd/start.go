package main

import (
    "dave/product/ipfs/ipfs_sniffer"
    "os"
    "os/signal"
    "syscall"
    "fmt"
)

func main() {
    ipfs_sniffer.IPFSSnifferBooting()
    
    termChan := make(chan os.Signal, 1)
    signal.Notify(termChan, os.Interrupt, syscall.SIGTERM)
    <-termChan // Blocks here until either SIGINT or SIGTERM is received.
    fmt.Println("Received interrupt signal, shutting down...")
}
