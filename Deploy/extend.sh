#!/bin/bash
#/*
# * Copyright (c) 2025 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
ACTION=$1
EXTEND=$2

MAC=$(ip addr | grep ether | awk '{print $2}' | head -n 1)

# example
# if [ "$MAC" == "xx:xx:xx:xx:xx:xx" ]; then
    # proxy="http://192.168.10.8:32475"
    # noproxy="localhost,127.0.0.1,::1,192.168.0.0/16,10.0.0.0/8"

    # if [ "$ACTION" == "build" ]; then
    #     if [ -z "$EXTEND" ]; then
    #         EXTEND="--build-arg http_proxy=$proxy --build-arg https_proxy=$proxy --build-arg no_proxy=$noproxy"
    #     else
    #         EXTEND="$EXTEND --build-arg http_proxy=$proxy --build-arg https_proxy=$proxy --build-arg no_proxy=$noproxy"
    #     fi
    # fi
    # if [ "$ACTION" == "run" ]; then
    #     if [ -z "$EXTEND" ]; then
    #         EXTEND="-e http_proxy=$proxy -e https_proxy=$proxy -e no_proxy=$noproxy"
    #     else
    #         EXTEND="$EXTEND -e http_proxy=$proxy -e https_proxy=$proxy -e no_proxy=$noproxy"
    #     fi
    # fi
# fi

echo "$EXTEND"