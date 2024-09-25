#!/bin/bash

CONTAINER=$1

docker stop ${CONTAINER}
docker rm ${CONTAINER}

docker image prune -a

# Clean cache
docker builder prune