#!/usr/bin/expect -f

# parse the arguments
set qemu  "./obj/qemu/i386-softmmu/qemu"
set img "~/tmp/liuran-os-lab5/obj/kern/bochs.img"
set img2 "~/tmp/liuran-os-lab5/obj/fs/fs.img"
set runmode [ lindex $argv 0 ]

# start a gdb
spawn sudo gdb
expect "(gdb) "

send "file $qemu\r"

send "handle SIG35 nostop noprint\r"
send "handle SIG36 nostop noprint\r"
send "handle SIG37 nostop noprint\r"
send "handle SIG38 nostop noprint\r"
#send "handle SIGINT pass\r"

#expect "Are you sure you want to change it? (y or n) "
#send "y\r"
send "set args -net none \
    -runmode $runmode \
	-smp 1 \
	-nographic \
	-serial mon:/dev/tty \
	-hda $img \
	-hdb $img2 \
	-m 1024 \r"
	#-bios seabios.bin \r"
	#-kernel $kernel \

# give back the control to user
interact
