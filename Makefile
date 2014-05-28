
# on a MAC
DISK="/Volumes/DISK Image"

CONFIG=qemu

all:
	make compile && make disk && make run

#assumes you have done source env.sh, or have a 'mk' wrapper that does that
compile:
	cd sys/src; mk all; cd cmd; mk install
	cd sys/src/9/pc; mk; mk install

disk:
	umount -f $(DISK) || echo not mounted
	open dosdisk.img
	sleep 3
	rm -rf $(DISK)/*
	cp -a ROOT/* $(DISK)/
	umount -f $(DISK)

run:
	qemu-system-i386 -smp 4 -m 256 \
           -kernel sys/src/9/pc/9$(CONFIG) \
           -hda dosdisk.img 
#-fda ~/floppy.img
#-hda ~/plan9.raw.img
#-cdrom plan9.iso? does not work?

clean:
	cd sys/src; mk clean
	cd sys/src/9/pc; mk clean


visual:
	~/pfff/codemap.opt -no_legend -no_symlinks -filter cpp -ss 2 .

graph:
	~/pfff/codegraph -derived_data -lang clang2 -build .

#todo? add libc, lib_networking, lib_memlayer, lib_memdraw, lib_draw, libmp?
graph2:
	~/pfff/codegraph -derived_data -lang clang2 -build include/ kernel/
check2:
	~/pfff/scheck -lang clang2 .

#trace:
#	mk clean
#	mk libs > make_trace.txt
#	mk kernels >> make_trace.txt
#	mk cmds >> make_trace.txt
#

# take quite some time :(
clangfiles:
	~/pfff/pfff -gen_clang compile_commands.json
	~/pfff/pfff_test.opt -uninclude_clang .
	cp sys/src/9/pc/apbootstrap.h kernel/init/386/
	mv sys/src/9/pc/apbootstrap.h.clang2 kernel/init/386/
	cp sys/src/9/pc/init.h kernel/init/user/preboot
	mv sys/src/9/pc/init.h.clang2 kernel/init/user/preboot
	cp sys/src/9/pc/reboot.h kernel/init/386
	mv sys/src/9/pc/reboot.h.clang2 kernel/init/386/
	cp sys/src/9/pc/$(CONFIG).c kernel/conf
	cp sys/src/9/pc/$(CONFIG).rootc.c kernel/conf
	mv sys/src/9/pc/*.clang2 kernel/conf
#	~/pfff/pfff_test -analyze_make_trace make_trace.txt > compile_commands.json

clangclean:
	find -name "*.clang*" -exec rm -f {} \;
