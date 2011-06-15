if [[ $# < 3 || $# > 4 ]]; then
    echo "Usage: $0 <num of core> <img> <runmode> [serial]"
    exit 1
fi

#qemu=~/local/bin/qemu-system-x86_64
qemu=obj/qemu/x86_64-softmmu/qemu-system-x86_64
#qemu=bin/bin/qemu-system-x86_64
#img=~/linux-img/sn-arch.qcow2
#img=~/linux-img/i686-arch.img
cores=$1
img=$2
runmode=$3

if [[ $# == 4 ]]; then
    serial=$4
else
    serial="mon:/dev/tty"
fi

memsize=1024

sudo $qemu \
    -smp $cores \
    -net none \
    -hda $img \
    -m $memsize \
    -k en-us \
    -nographic \
    -bios bin/share/qemu/seabios.bin \
    -runmode $runmode \
    -serial $serial \
    #-d in_asm,op \
    #-s \
