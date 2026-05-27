#!/bin/bash
#/*
# * Copyright (c) 2024 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

docker stop frontend
docker rm frontend

docker stop stats
docker rm stats

docker stop backend
docker rm backend

docker stop db
docker rm db

docker stop stats-db
docker rm stats-db

docker stop redis-db
docker rm redis-db

docker stop visualizer
docker rm visualizer

docker stop sig-provider
docker rm sig-provider

docker image prune -a