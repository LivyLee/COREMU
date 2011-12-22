#!/bin/bash

SESSIONDIR=`pwd`/oprofile

if [[ $# != 1 ]]; then
    echo "need to specify the command"
    exit 1
fi

cmd=$1

case $cmd in
    start)
        sudo opcontrol --reset
        sudo opcontrol --vmlinux=/boot/vmlinuz-2.6.32-5-amd64 --session-dir=$SESSIONDIR
        sudo opcontrol --start --session-dir=$SESSIONDIR
        ;;
    stop)
        sudo opcontrol --shutdown
        ;;
    report)
        opreport --session-dir=$SESSIONDIR -l obj/qemu/x86_64-softmmu/qemu-system-x86_64
        ;;
esac
