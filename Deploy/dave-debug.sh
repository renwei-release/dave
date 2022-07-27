#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

ldconfig
if [ -f "/project/public/base/lib/libjemalloc.so" ]; then
   export LD_PRELOAD="/project/public/base/lib/libjemalloc.so"
fi

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
    loop_notify "container on debug mode!"
}

goto_debug #___FLAG_FOR_UPDATE.SH___
