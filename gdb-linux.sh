#!/usr/bin/expect -f

# send user the usage
send_user "(usage) gdb-corey path2qemu path2img"

set imgdir "/home/alex/linux-img/"
set hda "$imgdir/arch.img"
set hdaqcow "$imgdir/arch.qcow2"
spawn sudo qemu-img create -f qcow2 -b $hda $hdaqcow
send_user "image for replay created"

# parse the arguments
set qemu  "./obj/qemu/x86_64-softmmu/qemu-system-x86_64"
#set qemu  "qemu-system-x86_64"
#set img "~/linux-img/debian-bench.img"
set runmode [ lindex $argv 0 ]
#send_user "$pid"

# start a gdb
spawn sudo cgdb
expect "(gdb) "

send "file $qemu\r"

send "handle SIG35 nostop noprint\r"
send "handle SIG36 nostop noprint\r"
send "handle SIG37 nostop noprint\r"
send "handle SIG38 nostop noprint\r"
#send "handle SIGINT pass\r"

#expect "Are you sure you want to change it? (y or n) "
#send "y\r"
send "set args \
    -net none \
	-nographic \
    -k en-us \
	-smp 1 \
	-hda $hdaqcow \
    -runmode $runmode \
    -bios bin/share/qemu/seabios.bin \
	-m 1024 \r"
	#-serial mon:/dev/tty \
	#-kernel $kernel \

# give back the control to user
interact
