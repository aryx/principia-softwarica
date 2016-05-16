TOP=..

all:
	echo todo

graph:
	echo "!!Adjust also the skip list!!"
	cd ..; ~/pfff/codegraph -lang c -build include/ windows/rio windows/libcomplete windows/libframe windows/libplumb

include $(TOP)/docs/latex/Makefile.common

##############################################################################
# Literate Programming rules
##############################################################################

DOC=Windows

# set of noweb (.nw) files

# must be in the same order of the #include for syncweb multi files support
# to work
SRC_ORIG=Windows.nw Windows_extra.nw

# from list_windows originally
SRC_VIEWS= \
   ../include/strings/complete.h\
   ../include/gui/frame.h\
   ../include/ipc/plumb.h\
   rio/dat.h\
   rio/globals.c\
   rio/data.c\
   rio/fns.h\
   rio/fsys.c\
   rio/rio.c\
   rio/scrl.c\
   rio/time.c\
   rio/util.c\
   rio/wctl.c\
   rio/wind.c\
   rio/xfid.c\
   ../applications/lens.c\
   ../applications/statusbar.c\
   ../applications/winwatch.c\
   libcomplete/complete.c\
   libframe/frbox.c\
   libframe/frdelete.c\
   libframe/frdraw.c\
   libframe/frinit.c\
   libframe/frinsert.c\
   libframe/frptofchar.c\
   libframe/frselect.c\
   libframe/frstr.c\
   libframe/frutil.c\
   libplumb/event.c\
   libplumb/mesg.c\
   libplumb/plumbsendtext.c\

#   ../lib_graphics/libmemdraw/hwdraw.c\
   ../kernel/devices/screen/386/cga.c\
$   codegraph.h

