#!/bin/bash
#/*
# * Copyright (c) 2024 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

if [ ! -d "account" ]; then
  mkdir account
fi

./bin/geth account new --datadir ./account