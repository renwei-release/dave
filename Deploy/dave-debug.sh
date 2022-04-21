#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

export LD_PRELOAD="./public/base/lib/libjemalloc.so ./public/base/lib/libjson-c.so"

function loop_notify()
{
    while true
    do
        echo "$1"
        read reply leftover
        case $reply in
            boot | reboot)
                return
                ;;
        esac
    done
}

function goto_debug()
{
    jupyter_booting
    loop_notify "This is an empty container!"
}

goto_debug #___FLAG_FOR_UPDATE.SH___
