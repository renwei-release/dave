#!/bin/bash
#/*
# * Copyright (c) 2024 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

#
#
# Pay attention to modifying the hardhat.config.js
# file to configure the relevant information of the
# chain where the contract is deployed
#
# url is RPC endpoint
# accounts is the private key of the account
#
#
#require("@nomicfoundation/hardhat-toolbox");
#
#/** @type import('hardhat/config').HardhatUserConfig */
#module.exports = {
#  solidity: "0.8.24",
#  networks: {
#    hk852network: {
#      url: "http://47.238.205.52:8545",
#      accounts: ["5cc85b***************************************************e8ff4"],
#    },
#  },
#};
#
#

modules_name=$1
if [ -z $modules_name ]; then
    modules_name="Lock"
fi

npx hardhat compile

echo "Deploying $modules_name contract to hk852network"

npx hardhat ignition deploy ./ignition/modules/$modules_name.js --network hk852network