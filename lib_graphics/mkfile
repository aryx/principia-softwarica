TOP=..

all:
	echo todo

include $(TOP)/docs/latex/Makefile.common

##############################################################################
# Literate Programming rules
##############################################################################

DOC=Graphics

# set of noweb (.nw) files

# must be in the same order of the #include for syncweb multi files support
# to work
SRC_ORIG=Graphics.nw Graphics_extra.nw

L1= ../include/graphics/draw.h\
    ../include/graphics/window.h\
    ../include/graphics/internals/font.h\
    ../include/graphics/internals/marshal.h\
    ../include/graphics/internals/draw_private.h\
   ../include/graphics/cursor.h\
   ../include/graphics/input/event.h\
   ../include/graphics/input/keyboard.h\
   ../include/graphics/input/mouse.h\
   \
   libdraw/misc.c\
   libdraw/alloc.c\
    libdraw/naming.c\
    libdraw/resolution.c\
   libdraw/arith.c\
   libdraw/bezier.c\
   libdraw/font.c\
   libdraw/chan.c\
   libdraw/debug.c\
   libdraw/fmt.c\
   libdraw/defont.c\
   libdraw/draw.c\
   libdraw/ellipse.c\
   libdraw/event.c\
   libdraw/egetrect.c\
   libdraw/emenuhit.c\
   libdraw/fontcache.c\
   libdraw/getrect.c\
   libdraw/icossin.c\
   libdraw/icossin2.c\
   libdraw/init.c\
    libdraw/error.c\
    libdraw/flush.c\
    libdraw/globals.c\
    libdraw/concurrency.c\
   libdraw/keyboard.c\
   libdraw/line.c\
   libdraw/menuhit.c\
   libdraw/mouse.c\
   libdraw/poly.c\
   libdraw/replclipr.c\
   libdraw/string.c\
   libdraw/stringwidth.c\
   libdraw/subfont.c\
   libdraw/window.c\
    libdraw/desktop.c\
   libdraw/io.c\
   libdraw/transfer.c\
   libdraw/compressed.c\
   libdraw/colormap.c\

L1TEST= libdraw/tests/drawtest.c


L2= ../include/graphics/internals/memdraw.h\
   \
   libmemdraw/alloc.c\
    libmemdraw/resolution.c\
    libmemdraw/chan.c\
   libmemdraw/arc.c\
   libmemdraw/compressed.c\
   libmemdraw/cmap.c\
   libmemdraw/defont.c\
   libmemdraw/draw.c\
    libmemdraw/init.c\
    libmemdraw/color.c\
    libmemdraw/alphadraw.c\
   libmemdraw/ellipse.c\
   libmemdraw/fillpoly.c\
   libmemdraw/hooks.c\
   libmemdraw/line.c\
   libmemdraw/transfer.c\
   libmemdraw/io.c\
   libmemdraw/poly.c\
   libmemdraw/string.c\
   libmemdraw/subfont.c\

L2TEST= \
   libmemdraw/scripts/mkcmap.c\
   libmemdraw/tests/arctest.c\
   libmemdraw/tests/memdrawtest.c\

#   ../lib_graphics/libmemdraw/hwdraw.c\

L3= ../include/graphics/internals/memlayer.h\
   \
   libmemlayer/draw.c\
   libmemlayer/lalloc.c\
   libmemlayer/layerop.c\
   libmemlayer/ldelete.c\
   libmemlayer/lhide.c\
   libmemlayer/line.c\
   libmemlayer/load.c\
   libmemlayer/lorigin.c\
   libmemlayer/lsetrefresh.c\
   libmemlayer/ltofront.c\
   libmemlayer/ltorear.c\
   libmemlayer/unload.c\

L4= \
   ../kernel/devices/screen/devdraw.c\
    ../kernel/devices/screen/devdraw.h\
    ../kernel/devices/screen/drawinit.c\
    ../kernel/devices/screen/drawname.c\
    ../kernel/devices/screen/drawalloc.c\
    ../kernel/devices/screen/drawwindow.c\
    ../kernel/devices/screen/drawerror.c\
    ../kernel/devices/screen/drawmesg.c\
    ../kernel/devices/screen/drawmisc.c\
   ../kernel/devices/screen/screen.h\
   ../kernel/devices/screen/screen.c\
   ../kernel/devices/screen/386/vga.h\
   ../kernel/devices/screen/386/vgascreen.c\
   ../kernel/devices/screen/386/devvga.c\
   ../kernel/devices/screen/386/vgaclgd542x.c\
   ../kernel/devices/screen/386/vgavesa.c\

#   ../kernel/devices/screen/386/cga.c\
#   codegraph.h

#not anymore, merged in vgascreen.c
#   ../kernel/devices/screen/386/vga.c\
#   ../kernel/devices/screen/386/vgax.c\

L5= \
   ../applications/clock.c\
   ../applications/colors.c\
   ../applications/screenlock.c\


SRC_VIEWS= $(L1) $(L1TEST) $(L2) $(L2TEST) $(L3) $(L4) $(L5)

loc1:
	wc -l $(L1)
loc2:
	wc -l $(L2)
loc3:
	wc -l $(L3) $(L4)


