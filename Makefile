#############################################################################
# Configuration section
#############################################################################
# on a MAC
DISK="/Volumes/DISK Image"

CONFIG=qemu

##############################################################################
# Top rules
##############################################################################

all:
	make compile && make disk && make run

# You'll need also kencc and plan9port binaries in PATH
setup_host:
	sudo ln -s `pwd`/root/386 /386
	sudo ln -s `pwd`/root/arm /arm
	sudo ln -s `pwd`/root/lib /lib
	sudo ln -s `pwd`/sys /sys
# this last one will be problematic on linux

# This assumes you have done source env.sh, or have a good 'mk' wrapper
compile:
	cd ROOT && mk
	cd sys/src && mk all
	cd sys/src/cmd && mk install
	cd sys/src/9/pc && mk && mk install
	mkdir -p ROOT/386/bin/ape/
	cp ROOT/386/bin/rc ROOT/386/bin/ape/sh

disk:
	umount -f $(DISK) || echo not mounted
	open dosdisk.img
	sleep 3
	rm -rf $(DISK)/*
	cp -a ROOT/* $(DISK)/
	umount -f $(DISK)

run:
	qemu-system-i386 -smp 4 -m 512 \
           -kernel sys/src/9/pc/9$(CONFIG) \
           -hda dosdisk.img \
#           -net nic,model=ne2k_pci -net user
#-fda ~/floppy.img
#-hda ~/plan9.raw.img
#-cdrom plan9.iso? does not work?
#-vga cirrus

clean:
	cd sys/src; mk clean
	cd sys/src/9/pc; mk clean
	cd ROOT; mk clean

clean_pfff:
	mv PFFF_* *.marshall *.opti layer* pfff.log .pfff/


##############################################################################
# Developer rules
##############################################################################

# codemap.opt has some issue with light db and graph db loading :(
visual:
	~/pfff/codemap.opt -no_legend -symlinks -filter xix -ss 2 .
graph:
	~/pfff/codegraph -derived_data -lang c -build .
tags:
	~/pfff/codegraph -derived_data -lang c -build .
check:
	~/pfff/scheck -filter 3 -lang c .


#todo? add libc, lib_networking, lib_memlayer/lib_memdraw/lib_draw, libmp?
KERNELSRC=include/ kernel/ lib_core/libc/9syscall lib_graphics/
# just kernel, but with new graph_code_c!
graph_kernel:
	~/pfff/codegraph -derived_data -lang c -build $(KERNELSRC)
graph_kernel2:
	~/pfff/codegraph -derived_data -lang c -build include/ kernel/ lib_core/libc/9syscall
prolog_kernel:
	~/pfff/codequery -lang c -build $(KERNELSRC)
datalog_kernel:
	~/pfff/codequery -datalog -lang c -build $(KERNELSRC)
loc_kernel:
	~/pfff/codemap -filter cpp -test_loc kernel

WINDOWSRC=include/ lib_graphics/ windows/ kernel/devices/screen/
graph_windows:
	~/pfff/codegraph -derived_data -lang c -build $(WINDOWSRC)
graph_windows2:
	~/pfff/codegraph -derived_data -lang c -build include/ lib_graphics/ windows/


##############################################################################
# Literate Programming rules
##############################################################################

NWDIRS=kernel windows shells \
       assemblers compilers linkers\
       builders debuggers profilers generators
#TODO: lib (libc, fmt, libthread, malloc, libregexp, libbio, libstring, ...)
#LATER: network/ security/

sync:
	set -e; for i in $(NWDIRS); do $(MAKE) -C $$i sync || exit 1; done 

# take care!
lpdistclean:
	set -e; for i in $(NWDIRS); do $(MAKE) -C $$i lpdistclean || exit 1; done 

visual2:
	~/pfff/codemap.opt -no_legend -symlinks -filter nw -ss 2 .


#	cp sys/src/9/pc/apbootstrap.h kernel/init/386/
#	cp sys/src/9/pc/init.h kernel/init/user/preboot
#	cp sys/src/9/pc/reboot.h kernel/init/386
#	cp sys/src/9/pc/$(CONFIG).c kernel/conf
#	cp sys/src/9/pc/$(CONFIG).rootc.c kernel/conf
#	mv sys/src/9/pc/*.clang2 kernel/conf
