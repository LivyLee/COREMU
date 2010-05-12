echo "(usage) run-linux.sh path2qemu path2img" 

sudo $1 \
    -serial stdio \
    -smp 128 \
    -net none \
    -hda $2 \
    -m 8192 \
    -k en-us \
    -nographic \
    -bios seabios.bin \
