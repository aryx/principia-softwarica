<mkconfig

###############################################################################
# Libs
###############################################################################

LIBS=\
 lib_core\
 lib_strings\
 lib_math\
 lib_networking\
 lib_graphics\
 # used by plumb, iostats, snapfs, screenlock, etc\
 lib_security\
 lib_misc\
 \
 generators/lex/liblex\
 debuggers/libmach\
 # used by networking/ip, aux/vga/db, etc.\
 database/libndb\
 # used by compilers/cpp (which is used by pcc), languages/s9\
 MISC/APE/libstdio\

#TODO: 
# lib_audio (empty)
# lib_gui (need to adapt to new draw using view not screen)

###############################################################################
# Programs
###############################################################################

PROGRAMS=\
 assemblers\
 linkers\
 compilers\
 generators\
 machine\
 MISC/APE/pcc\
 \
 builders\
 debuggers\
 profilers\
 editors\
 \
 shells\
 windows\
 \
 networking\
 \
 utilities\
 \
 applications\
 games

#missing:
# browsers
# interpreters/s9
# security/auth/

#missing in subdirs:
# networking/http
# graphics/libimg
# ...

TESTS=\
 lib_graphics/libdraw/tests\
 lib_graphics/libmemdraw/tests\
 compilers/8c/tests\
 linkers/8l/tests\
 # need a arm/lib/libc.a\
 # compilers/5c/tests\

###############################################################################
# Kernel
###############################################################################

# those programs (e.g., bind, mount) are used as an initial /bin/
# and mentioned in boot.rc
BOOTCMDS=\
 kernel/files/user\
 kernel/devices/storage/user/386/libdisk\
 kernel/devices/storage/user/386/prep\
 kernel/filesystems/user/dossrv

CMDSTODO=\
 kernel/filesystems/user/ramfs\
 kernel/devices/mouse/user\
 kernel/devices/screen/user/386\
 kernel/devices/sys/user\
 \
 # requires (slow) pcc, too slow so skipped for now\
 typesetting\

# kernel/devices/storage/user
# kernel/init/user
# kernel/memory/user
# kernel/syscalls/user

KERNELS=kernel/COMPILE/9/pc 

###############################################################################
# Targets
###############################################################################
CMDS=$PROGRAMS $BOOTCMDS

DIRS=$LIBS $CMDS #$KERNELS

all:QV:
	for (i in $DIRS) @{
		cd $i
		mk $MKFLAGS $target
	}

install uninstall clean nuke:QV:
	for (i in $DIRS) @{
		cd $i
		mk $MKFLAGS $target
	}

help:VQ:
	echo mk all, install, uninstall, clean, nuke, release, cmds, kernels, or libs

installall:V:
	echo "installall not supported"


libs:QV:
	for (i in $LIBS) @{
		cd $i
		mk $MKFLAGS install
	}

cmds:QV:
	for (i in $CMDS) @{
        cd $i
		mk $MKFLAGS install
    }

kernels:QV:
	for (i in $KERNELS) @{
		cd $i
		mk $MKFLAGS install
	}

<mkfile-host-$HOST
<mkfile-target-$TARGET
