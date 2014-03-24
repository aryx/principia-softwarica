
DISK="/Volumes/DISK Image"

all:
	make compile && make disk && make run

#assumes you have done source env.sh, or have a 'mk' wrapper that does that
compile:
	cd sys/src; mk all; cd cmd; mk install

disk:
	umount -f $(DISK) || echo not mounted
	open dosdisk.img
	sleep 3
	rm -rf $(DISK)/*
	cp -a root/* $(DISK)/
	umount -f $(DISK)


run:
	qemu-system-i386 -smp 4 -m 256 \
           -kernel sys/src/9/pc/9pcf \
           -hda dosdisk.img 

#-fda ~/floppy.img
#-hda ~/plan9.raw.img
# -cdrom plan9.iso? does not work?


	(cd sys/src/9/pc/; make qemu)


visual:
	~/pfff/codemap.opt -no_legend -no_symlinks -filter cpp -ss 2 .

graph:
	~/pfff/codegraph -derived_data -lang clang2 -build .

#include also libc, lib_networking, lib_memlayer, lib_memdraw, lib_draw, libmp?
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
	cp sys/src/9/pc/apbootstrap.h kernel/conf
	cp sys/src/9/pc/init.h kernel/conf
	cp sys/src/9/pc/reboot.h kernel/conf
	cp sys/src/9/pc/pcf.c kernel/conf
	cp sys/src/9/pc/pcf.rootc.c kernel/conf
	cp sys/src/9/pc/errstr.c kernel/core
	cp sys/src/9/pc/*.clang2 kernel/conf
	mv kernel/conf/errstr.c.clang2 kernel/core
#	~/pfff/pfff_test -analyze_make_trace make_trace.txt > compile_commands.json

clangclean:
	find -name "*.clang*" -exec rm -f {} \;
