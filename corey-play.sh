#!/bin/bash

imgdir=~/coremu/mit-corey/obj
hda=$imgdir/boot/bochs.img
hdaqcow=$imgdir/boot/play-bochs.qcow2
hdb=$imgdir/fs/fs.fat
hdbqcow=$imgdir/fs/play-fs.qcow2

sudo qemu-img create -f qcow2 -b $hda $hdaqcow
sudo qemu-img create -f qcow2 -b $hdb $hdbqcow
echo image for replay created
./corey.sh 4 $hdaqcow $hdbqcow replay $1
cp /tmp/qemu.log replay-log/qemu.log-replay
