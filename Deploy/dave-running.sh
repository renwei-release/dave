#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

action=release # debug release test

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

function jupyter_booting()
{
    if command -v jupyter > /dev/null 2>&1; then
        jupyter notebook --allow-root &
    fi
}

function goto_debug()
{
    jupyter_booting
    loop_notify "This is an empty container!"
}

hasflagnum=$(grep -c "FLAG_FOR_UPDATE" ${0##*/})
if [ $hasflagnum == 2 ]; then
    goto_debug
fi

if [ "$action" = "release" ];then
goto_debug #___FLAG_FOR_UPDATE.SH___
else
    goto_debug
fi