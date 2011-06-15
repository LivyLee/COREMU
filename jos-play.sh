#!/bin/bash

./jos.sh replay $1
cp /tmp/qemu.log qemu.log-replay
