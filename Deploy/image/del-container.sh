#!/bin/bash

CONTAINER=$1

docker stop ${CONTAINER}
docker rm ${CONTAINER}

chmod +x ./prune-image.sh && ./prune-image.sh