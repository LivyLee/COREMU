if [[ $# == 0 || $# > 2 ]]; then
    echo "Usage: $0 <runmode> [serial]"
    exit 1
fi

josdir=~/coremu/jos

qemu=obj/qemu/i386-softmmu/qemu
#qemu=~/coremu/upstream-qemu/i386-softmmu/qemu
#qemu=~/local/bin/qemu
img=$josdir/obj/kern/kernel.img
cores=1
runmode=$1

if [[ $# == 2 ]]; then
    serial=$2
else
    serial="mon:/dev/tty"
fi

memsize=512

rm -f log/*

sudo $qemu \
    -smp $cores \
    -net none \
    -hda $img \
    -m $memsize \
    -k en-us \
    -nographic \
    -bios bin/share/qemu/seabios.bin \
    -d in_asm,int \
    -runmode $runmode \
    -serial $serial \
    #-s \
