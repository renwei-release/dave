#!/bin/bash
#/*
# * Copyright (c) 2022 Chenxiaomin
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

CONTAINER=${PWD##*/}

docker exec -it ${CONTAINER}-log bash