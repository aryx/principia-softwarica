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


LIBSTODO=\
 # used by cmd/disk\
 kernel/devices/storage/user/386/libdisk\
 # TODO empty directory for now\
 lib_audio\
 # TODO need to adapt to new draw using view not screen\
 lib_gui\

###############################################################################
# Programs
###############################################################################

CMDS=\
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


CMDSTODO=\
 kernel/files/user\
 kernel/filesystems/user/dossrv\
 kernel/filesystems/user/ramfs\
 kernel/devices/mouse/user\
 kernel/devices/screen/user/386\
 kernel/devices/sys/user\
 \
 # requires (slow) pcc, too slow so skipped for now\
 typesetting\
 browsers\

# interpreters/s9
# networking/http
# graphics/jpg
# security: auth/
# kernel/devices/storage/user
# kernel/init/user
# kernel/memory/user
# kernel/syscalls/user

TESTS=\
 lib_graphics/libdraw/tests\
 lib_graphics/libmemdraw/tests\
 compilers/8c/tests\
 linkers/8l/tests\
 # need a arm/lib/libc.a\
 # compilers/5c/tests\

KERNELS=kernel/COMPILE/9/pc 

DIRS=$LIBS $CMDS

###############################################################################
# Targets
###############################################################################

all:QV:
	for (i in $DIRS) @{
		cd $i
		mk $MKFLAGS $target
	}

install clean nuke:QV:
	for (i in $DIRS) @{
		cd $i
		mk $MKFLAGS $target
	}

help:VQ:
	echo mk all, install, clean, nuke, release, cmds, kernels, or libs

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
