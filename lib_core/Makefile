TOP=..

all:
	echo todo

include $(TOP)/docs/latex/Makefile.common

##############################################################################
# Literate Programming rules
##############################################################################

TEXMAIN=Libcore.nw
TEX=Libcore.tex

# set of noweb (.nw) files

# must be in the same order of the #include for syncweb multi files support
# to work
SRC_ORIG=Libcore.nw Libcore_extra.nw

# from list_graphics originally
SRC_VIEWS= \
   ../include/ipc/thread.h\
   libthread/dial.c\
   libthread/ref.c\
   libthread/threadimpl.h\
   libthread/xincarm.c\
   libthread/xincport.h\
   libthread/globals.c\
   libthread/debug.c\
   libthread/id.c\
   libthread/exit.c\
   libthread/lib.c\
   libthread/note.c\
   libthread/rendez.c\
   libthread/386.c\
   libthread/sched.c\
   libthread/create.c\
   libthread/channel.c\
   libthread/main.c\
   libthread/kill.c\
   libthread/ioproc.c\
   libthread/iocall.c\
   libthread/chanprint.c\
   libthread/ioclose.c\
   libthread/iodial.c\
   libthread/ioopen.c\
   libthread/ioread.c\
   libthread/ioreadn.c\
   libthread/iosleep.c\
   libthread/iowrite.c\
   libthread/exec.c\

# libthread/arm.c
# libthread/xinc386.s
# tests/