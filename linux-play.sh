#!/bin/bash

imgdir=~/linux-img

sudo qemu-img create -f qcow2 -b $imgdir/arch.img $imgdir/play-arch.qcow2
echo image for replay created
./linux.sh 1 $imgdir/play-arch.qcow2 replay $1
cp /tmp/qemu.log replay-log/qemu.log-replay
