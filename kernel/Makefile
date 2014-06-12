
all:
	make sync

visual:
	~/pfff/codemap -ss 2 -no_legend -filter nw -ft 0.001 -nblines_file 60000 .

##############################################################################
# Generic Literate programming variables
##############################################################################

SYNCFLAGS=-md5sum_in_auxfile -less_marks -lang C

SYNCWEB=~/github/syncweb/syncweb.opt $(SYNCFLAGS)
NOWEB=~/github/syncweb/scripts/noweblatex
PDFLATEX=pdflatex --shell-escape 

lpclean::
	rm -f *.aux *.toc *.log *.brf *.out

##############################################################################
# Literate Programming rules
##############################################################################

TEXMAIN=Kernel.tex.nw
TEX=Kernel.tex

# set of noweb (.nw) files

# must be in the same order of the #include for syncweb multi files support
# to work
SRC_ORIG=Kernel.tex.nw Kernel_extra.tex.nw

SRC_VIEWS= \
  ../lib_core/libc/9syscall/sys.h\
  ../include/trace.h\
  ../include/tos.h\
  arch/386/arch.c \
  arch/386/bios32.c \
  arch/386/dat_arch.h \
  arch/386/io.h \
  arch/386/iomap.c \
  arch/386/l_io.s \
  arch/386/l_realmode.s \
  arch/386/mp.h \
  arch/386/mpacpi.h \
  arch/386/nvram.c \
  arch/386/realmode.c \
  arch/386/x16.h \
  arch/386/x86.c \
  buses/386/dat_buses.h \
  buses/386/dma.c \
  buses/386/pci.c \
  buses/386/uarti8250.c \
  buses/386/uartisa.c \
  buses/devuart.c \
  buses/portdat_buses.h \
  buses/portfns_buses.h \
  concurrency/386/l_concurrency.s \
  concurrency/portdat_concurrency.h \
  concurrency/portfns_concurrency.h \
  concurrency/qlock.c \
  concurrency/ref.c \
  concurrency/taslock.c \
  console/devcons.c \
  console/portdat_console.h \
  console/portfns_console.h \
  console/print.c \
  console/rdb.c \
  core/386/dat.h \
  core/386/dat_core.h \
  core/386/dat_forward.h \
  core/386/fns.c \
  core/386/fns.h \
  core/error.h \
  core/error.c \
  core/portdat.c \
  core/portdat_core.h \
  core/portdat_forward.h \
  core/portfns.c \
  core/portfns_core.h \
  devices/keyboard/386/kbd.c \
  devices/keyboard/386/latin1.c \
  devices/keyboard/386/latin1.h \
  devices/portfns_devices.h \
  devices/screen/386/cga.c \
  devices/storage/386/devfloppy.c \
  devices/storage/386/floppy.h \
  devices/storage/386/sdata.c \
  devices/storage/devsd.c \
  devices/storage/sd.h \
  devices/storage/sdscsi.c \
  devices/sys/386/devarch.c \
  devices/sys/386/devrtc.c \
  devices/sys/devdup.c \
  devices/sys/devenv.c \
  devices/sys/devmnt.c \
  devices/sys/devpipe.c \
  devices/sys/devproc.c \
  devices/sys/devroot.c \
  devices/sys/devsrv.c \
  devices/sys/devsys.c \
  files/allocb.c \
  files/cache.c \
  files/chan.c \
  files/dev.c \
  files/env.c \
  files/file.c \
  files/mnt.c \
  files/portdat_files.h \
  files/portfns_files.h \
  files/qio.c \
  files/sysfile.c \
  filesystems/devfs.c \
  init/386/apbootstrap.s \
  init/386/l.s \
  init/386/l_fp.s \
  init/386/l_misc.s \
  init/386/l_multiboot.s \
  init/386/main.c \
  init/386/rebootcode.s \
  init/user/preboot/initcode.c \
  init/user/preboot/386/init9.c \
  init/user/boot/aux.c \
  init/user/boot/boot.c \
  init/user/boot/boot.h \
  init/user/boot/local.c \
  init/portfns_init.h \
  init/rebootcmd.c \
  lib/lib.h \
  memory/386/dat_memory.h \
  memory/386/mem.h \
  memory/386/memory.c \
  memory/386/mmu.c \
  memory/alloc.c \
  memory/fault.c \
  memory/page.c \
  memory/pool.c \
  memory/portdat_memory.c \
  memory/portdat_memory.h \
  memory/portfns_memory.h \
  memory/segment.c \
  memory/swap.c \
  memory/sysmemory.c \
  memory/xalloc.c \
  misc/parse.c \
  misc/portdat_misc.h \
  misc/portfns_misc.h \
  misc/random.c \
  processes/386/apic.c \
  processes/386/archgeneric.c \
  processes/386/archmp.c \
  processes/386/dat_processes.h \
  processes/386/i8253.c \
  processes/386/i8259.c \
  processes/386/l_switch.s \
  processes/386/l_trap.s \
  processes/386/main_processes.c \
  processes/386/mp.c \
  processes/386/mpacpi.c \
  processes/386/trap.c \
  processes/alarm.c \
  processes/clock.c \
  processes/edf.c \
  processes/pgrp.c \
  processes/portdat_processes.c \
  processes/portdat_processes.h \
  processes/portfns_processes.h \
  processes/proc.c \
  processes/sysproc.c \
  processes/syssema.c \
  processes/tod.c \
  security/auth.c \
  security/portfns_security.h \
  syscalls/386/plan9l.s \
  syscalls/syscallfmt.c \
  syscalls/systab.c \
  syscalls/systab.h \


sync:
	for i in $(SRC_VIEWS); do echo $$i; $(SYNCWEB) $(SRC_ORIG) $$i || exit 1; done 

pdf:
	$(NOWEB) $(TEXMAIN) > $(TEX)
	pdflatex $(TEX)
	pdflatex $(TEX)

lpclean::
	rm -f $(TEX)

lpdistclean::
	rm -f $(SRC_VIEWS) .md5sum_* $(TEX)

clean::
	rm -f *.aux *.toc *.log *.brf *.out
