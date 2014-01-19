all:
	./compile.sh
	qemu-system-i386 -smp 4 -m 256 \
           -kernel 9pcf \
           -hda ~/xplan9.raw.img -hdb ~/dosdisk.img -fda ~/floppy.img

# -cdrom plan9.iso? does not work?


