#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

CONTAINER=${PWD##*/}

CMD=$1
PARAM=$2

#BASIC COMMANDS
#    init          Initialize local IPFS configuration
#    add <path>    Add a file to IPFS
#    cat <ref>     Show IPFS object data
#    get <ref>     Download IPFS objects
#    ls <ref>      List links from an object
#    refs <ref>    List hashes of links from an object
  
#  DATA STRUCTURE COMMANDS
#    dag           Interact with IPLD DAG nodes
#    files         Interact with files as if they were a unix filesystem
#    block         Interact with raw blocks in the datastore
  
#  TEXT ENCODING COMMANDS
#    cid           Convert and discover properties of CIDs
#    multibase     Encode and decode data with Multibase format
  
#  ADVANCED COMMANDS
#    daemon        Start a long-running daemon process
#    shutdown      Shut down the daemon process
#    resolve       Resolve any type of content path
#    name          Publish and resolve IPNS names
#    key           Create and list IPNS name keypairs
#    pin           Pin objects to local storage
#    repo          Manipulate the IPFS repository
#    stats         Various operational stats
#    p2p           Libp2p stream mounting (experimental)
#    filestore     Manage the filestore (experimental)
#    mount         Mount an IPFS read-only mount point (experimental)
  
#  NETWORK COMMANDS
#    id            Show info about IPFS peers
#    bootstrap     Add or remove bootstrap peers
#    swarm         Manage connections to the p2p network
#    dht           Query the DHT for values or peers
#    routing       Issue routing commands
#    ping          Measure the latency of a connection
#    bitswap       Inspect bitswap state
#    pubsub        Send and receive messages via pubsub
  
#  TOOL COMMANDS
#    config        Manage configuration
#    version       Show IPFS version information
#    diag          Generate diagnostic reports
#    update        Download and apply go-ipfs updates
#    commands      List all available commands
#    log           Manage and show logs of running daemon

if [ "$CMD" == "" ]; then
   CMD=id
fi

docker exec -it ${CONTAINER} ipfs ${CMD} ${PARAM}