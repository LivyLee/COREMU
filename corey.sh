#!/bin/sh

if [[ $# < 4 || $# > 5 ]]; then
    echo "Usage: $0 <num of core> <img1> <img2> <runmode> [serial]"
    exit 1
fi

qemu=obj/qemu/x86_64-softmmu/qemu-system-x86_64
cores=$1
hda=$2
hdb=$3
runmode=$4

if [[ $# == 5 ]]; then
    serial=$4
else
    serial="mon:/dev/tty"
fi

memsize=1024

sudo $qemu \
    -smp $cores \
    -net none \
	-hda $hda \
	-hdb $hdb \
    -m $memsize \
    -k en-us \
    -nographic \
    -bios bin/share/qemu/seabios.bin \
    -runmode $runmode \
    -serial $serial \
    #-d in_asm,op \
    #-s \
