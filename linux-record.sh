#!/bin/bash

imgdir=~/linux-img

sudo qemu-img create -f qcow2 -b $imgdir/arch.img $imgdir/record-arch.qcow2
echo image for recording created
./linux.sh 1 $imgdir/record-arch.qcow2 record $1
cp /tmp/qemu.log replay-log/qemu.log-record
