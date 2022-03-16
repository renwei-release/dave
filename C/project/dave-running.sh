#!/bin/bash

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

action=release

if [ "$action" = "release" ];then
___FLAG_FOR_UPDATE.SH___
else
loop_notify "This is an empty container!"
fi