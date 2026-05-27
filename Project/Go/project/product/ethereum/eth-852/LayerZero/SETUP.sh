#!/bin/bash
#/*
# * Copyright (c) 2024 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

home_dir=$(eval echo ~${SUDO_USER})

echo the home: $home_dir

if [ ! -d $home_dir/.foundry ]; then
    sudo apt update
    sudo apt install git -y
    sudo apt install curl -y
    sudo apt install npm -y

    curl -L https://foundry.paradigm.xyz | bash
    source $home_dir/.bashrc
    foundryup
fi

if [ ! -d LayerZero-v2 ]; then
    git clone https://github.com/LayerZero-Labs/LayerZero-v2.git
fi

echo LayerZero-v2 is ready to go!