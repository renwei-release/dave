#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

CONTAINER=${PWD##*/}
DATADIR=ethereum

docker exec -it ${CONTAINER} ./geth --datadir ${DATADIR} attach