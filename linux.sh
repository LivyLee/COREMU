#!/bin/bash

if [[ $# != 1 ]]; then
    echo "Usage: $0 <num of core>"
    exit 1
fi


qemu=obj/qemu/x86_64-softmmu/qemu-system-x86_64
#img=~/linux-img/arch.qcow2
#img=~/linux-img/debian-bench.qcow2
baseimg=~/linux-img/debian6.img
img=~/linux-img/debian6.qcow2
cores=$1

qemu-img create -f qcow2 -b $baseimg $img

memsize=1024

sudo $qemu \
    -smp $cores \
    -net none \
    -hda $img \
    -m $memsize \
    -k en-us \
    -serial mon:/dev/tty \
    -nographic \
    -bios bin/share/qemu/seabios.bin \
    #-s \
