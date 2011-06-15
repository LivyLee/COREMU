#!/bin/bash

imgdir=~/linux-img

sudo qemu-img create -f qcow2 -b $imgdir/arch.img $imgdir/record-arch.qcow2
echo image for recording created
./linux.sh 4 $imgdir/record-arch.qcow2 normal $1
cp /tmp/qemu.log replay-log/qemu.log-record
