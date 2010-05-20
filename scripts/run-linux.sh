echo "(usage) run-linux.sh path2qemu path2img" 

sudo $1 \
    -smp 1 \
    -net none \
    -hda $2 \
    -m 1024 \
    -k en-us \
    -nographic \
