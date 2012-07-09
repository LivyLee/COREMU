#!/usr/bin/expect -f

# send user the usage
send_user "(usage) gdb-corey path2qemu path2img"

# parse the arguments
set qemu  "./obj/qemu/x86_64-softmmu/qemu-system-x86_64"
#set qemu  "qemu-system-x86_64"
set img "./debian_bench.img"
set kernel "~/bzImage-2.6.33.1-debug"
set pid [ lindex $argv 0 ]
send_user "$pid"

# start a gdb
spawn sudo gdb

send "file $qemu\r"

send "handle SIG35 nostop noprint\r"
send "handle SIG36 nostop noprint\r"
send "handle SIG37 nostop noprint\r"
send "handle SIG38 nostop noprint\r"
#send "handle SIGINT pass\r"
send "attach $pid\r"

#expect "Are you sure you want to change it? (y or n) "
#send "y\r"
#send "b tcg.c:1576 \r"
#send "b helper_atomic_cmpxchgb \r"
#send "b helper_atomic_cmpxchgw \r"
#send "b helper_atomic_cmpxchgl \r"
#send "b helper_atomic_cmpxchgq \r"
#send "b qemu_realloc \r"
#send "r -net none \
#	-smp 128 \
#	-nographic \
#	-serial mon:/dev/tty \
#	-hda $img \
#	-m 8000 \
#	-bios seabios.bin \r"
	#-kernel $kernel \

# give back the control to user
interact
