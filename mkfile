<mkconfig

###############################################################################
# Libs
###############################################################################

LIBS=\
 lib_core\
 lib_math\
 lib_strings\
 lib_graphics\
 lib_gui\
 lib_networking\
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
 shells\
 windows\
 \
 builders\
 debuggers\
 profilers\
 editors\
 \
 networking\
 browsers\
 \
 utilities\
 \
 applications\
 games\

#missing:
# interpreters/s9
# security/auth/
# typesetting (requires (slow) pcc, too slow so skipped for now)

#missing in subdirs:
# networking/http
# graphics/libimg
# ...

# used by mk clean
TESTS=\
 ROOT/tests\
 lib_graphics/libdraw/tests\
 lib_graphics/libmemdraw/tests\
 compilers/8c/tests\
 linkers/8l/tests\
 lib_gui/libpanel/tests\

# assemblers/5a/tests\
# need a arm/lib/libc.a\
# compilers/5c/tests\

###############################################################################
# Kernel
###############################################################################

# the programs in the directories below (e.g., bind, mount) are used as 
# an initial /bin/ and mentioned in boot.rc
BOOTCMDS=\
 # those programs are part of the root image (thx to data2s)\
 kernel/files/user\
 kernel/devices/storage/user/386/libdisk\
 kernel/devices/storage/user/386/prep\
 kernel/filesystems/user/dossrv\
 # those programs are not part of the root image because they are called\
 # after the call to dossrv\
 kernel/devices/mouse/user\
 kernel/devices/screen/user/386\
 kernel/filesystems/user/ramfs\
 # mntgen and ns\
 kernel/devices/sys/user\
 # for pi\
 kernel/buses/user/usb

#TODO:
# kernel/memory/user
# kernel/syscalls/user

#This is set in mkfile-target-xxx
#KERNELDIR=kernel/COMPILE/9/pc 

###############################################################################
# Targets
###############################################################################
CMDS=$PROGRAMS $BOOTCMDS

DIRS=$LIBS $CMDS

all:QV:
	for (i in $DIRS) @{
		echo $i
		cd $i
		mk $MKFLAGS $target
	}

install uninstall:QV:
	for (i in $DIRS) @{
		echo $i
		cd $i
		mk $MKFLAGS $target
	}

clean nuke:QV:
	for (i in $DIRS $TESTS) @{
		echo $i
		cd $i
		mk $MKFLAGS $target
	}


help:VQ:
	echo mk all, install, uninstall, clean, nuke, cmds, libs, or kernel

libs:QV:
	for (i in $LIBS) @{
		echo $i
		cd $i
		mk $MKFLAGS install
	}

cmds:QV:
	for (i in $CMDS) @{
		echo $i
        cd $i
		mk $MKFLAGS install
    }

kernel:QV:
    # required to get the binaries for mkboot
	mk $MKFLAGS install
	for (i in $KERNELDIR) @{
		echo $i
		cd $i
		mk $MKFLAGS install
	}

disk:V: disk-$TARGET

doall:V:
	mk all
	mk kernel
	mk disk
    mk disksrc
    mk run

# this defines the disk-xxx targets
<mkfile-host-$HOST
# this defines the run target
<mkfile-target-$TARGET

###############################################################################
# Literate programming
###############################################################################
LPDIRS=\
 docs/principia \
 kernel shells lib_core\
 assemblers linkers compilers machine generators languages \
 editors builders debuggers profilers version_control \
 lib_graphics windows lib_gui \
 networking browsers

#TODO
# utilities
#LESS
# s9, tiger, soldat

pdf lpclean pdfinstall:QV:
	for (i in $LPDIRS) @{
		echo $i
		cd $i
		mk $MKFLAGS $target
	}

# need special :I: supported currently only by mk-in-ocaml
#sync:QVI:
#	for (i in $LPDIRS) @{
#		echo $i
#		cd $i
#		mk $MKFLAGS $target
#	}
