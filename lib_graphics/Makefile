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

# from list_graphics originally
SRC_VIEWS= \
   ../include/graphics/draw.h\
    ../include/graphics/window.h\
    ../include/graphics/internals/font.h\
    ../include/graphics/internals/marshal.h\
    ../include/graphics/internals/draw_private.h\
   \
   ../include/graphics/internals/memdraw.h\
   ../include/graphics/internals/memlayer.h\
   ../include/graphics/cursor.h\
   ../include/graphics/input/event.h\
   ../include/graphics/input/keyboard.h\
   ../include/graphics/input/mouse.h\
   \
   libdraw/misc.c\
   libdraw/alloc.c\
    libdraw/naming.c\
   libdraw/arith.c\
   libdraw/bezier.c\
   libdraw/buildfont.c\
   libdraw/bytesperline.c\
   libdraw/chan.c\
   libdraw/debug.c\
   libdraw/fmt.c\
   libdraw/defont.c\
   libdraw/draw.c\
   libdraw/ellipse.c\
   libdraw/event.c\
   libdraw/egetrect.c\
   libdraw/emenuhit.c\
   libdraw/font.c\
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
   libdraw/openfont.c\
   libdraw/poly.c\
   libdraw/replclipr.c\
   libdraw/scroll.c\
   libdraw/string.c\
   libdraw/stringwidth.c\
   libdraw/subfont.c\
   libdraw/subfontcache.c\
   libdraw/subfontname.c\
   libdraw/window.c\
    libdraw/desktop.c\
    libdraw/io.c\
    libdraw/transfer.c\
    libdraw/compressed.c\
    libdraw/colormap.c\
   libdraw/tests/drawtest.c\
   \
   libmemdraw/alloc.c\
   libmemdraw/arc.c\
   libmemdraw/cload.c\
   libmemdraw/cmap.c\
   libmemdraw/cread.c\
   libmemdraw/defont.c\
   libmemdraw/draw.c\
   libmemdraw/ellipse.c\
   libmemdraw/fillpoly.c\
   libmemdraw/iprint.c\
   libmemdraw/line.c\
   libmemdraw/load.c\
   libmemdraw/poly.c\
   libmemdraw/read.c\
   libmemdraw/scripts/mkcmap.c\
   libmemdraw/string.c\
   libmemdraw/subfont.c\
   libmemdraw/tests/arctest.c\
   libmemdraw/tests/memdrawtest.c\
   libmemdraw/unload.c\
   libmemdraw/write.c\
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
   \
   ../applications/clock.c\
   ../applications/colors.c\
   ../applications/screenlock.c\
   \
   ../kernel/devices/screen/devdraw.c\
   ../kernel/devices/screen/screen.h\
   ../kernel/devices/screen/screen.c\
   ../kernel/devices/screen/386/vga.h\
   ../kernel/devices/screen/386/vgascreen.c\
   ../kernel/devices/screen/386/devvga.c\
   ../kernel/devices/screen/386/vgaclgd542x.c\
   ../kernel/devices/screen/386/vgavesa.c\

#   ../lib_graphics/libmemdraw/hwdraw.c\
   ../kernel/devices/screen/386/cga.c\
$   codegraph.h

#not anymore, merged in vgascreen.c
#   ../kernel/devices/screen/386/vga.c\
#   ../kernel/devices/screen/386/vgax.c\

