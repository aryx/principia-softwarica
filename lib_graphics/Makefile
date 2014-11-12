TOP=..

all:
	echo todo

include $(TOP)/docs/latex/Makefile.common

##############################################################################
# Literate Programming rules
##############################################################################

TEXMAIN=Graphics.tex.nw
TEX=Graphics.tex

# set of noweb (.nw) files

# must be in the same order of the #include for syncweb multi files support
# to work
SRC_ORIG=Graphics.tex.nw Graphics_extra.tex.nw

# from list_graphics originally
SRC_VIEWS= \
   ../include/cursor.h\
   ../include/draw.h\
   ../include/event.h\
   ../include/keyboard.h\
   ../include/memdraw.h\
   ../include/memlayer.h\
   ../include/mouse.h\
   libdraw/alloc.c\
   libdraw/allocimagemix.c\
   libdraw/arith.c\
   libdraw/bezier.c\
   libdraw/border.c\
   libdraw/buildfont.c\
   libdraw/bytesperline.c\
   libdraw/chan.c\
   libdraw/cloadimage.c\
   libdraw/computil.c\
   libdraw/creadimage.c\
   libdraw/debug.c\
   libdraw/defont.c\
   libdraw/draw.c\
   libdraw/drawrepl.c\
   libdraw/egetrect.c\
   libdraw/ellipse.c\
   libdraw/emenuhit.c\
   libdraw/event.c\
   libdraw/fmt.c\
   libdraw/font.c\
   libdraw/freesubfont.c\
   libdraw/getdefont.c\
   libdraw/getrect.c\
   libdraw/getsubfont.c\
   libdraw/icossin.c\
   libdraw/icossin2.c\
   libdraw/init.c\
   libdraw/keyboard.c\
   libdraw/line.c\
   libdraw/loadimage.c\
   libdraw/menuhit.c\
   libdraw/mkfont.c\
   libdraw/mouse.c\
   libdraw/newwindow.c\
   libdraw/openfont.c\
   libdraw/poly.c\
   libdraw/readcolmap.c\
   libdraw/readimage.c\
   libdraw/readsubfont.c\
   libdraw/rectclip.c\
   libdraw/replclipr.c\
   libdraw/rgb.c\
   libdraw/scroll.c\
   libdraw/string.c\
   libdraw/stringbg.c\
   libdraw/stringsubfont.c\
   libdraw/stringwidth.c\
   libdraw/subfont.c\
   libdraw/subfontcache.c\
   libdraw/subfontname.c\
   libdraw/tests/drawtest.c\
   libdraw/unloadimage.c\
   libdraw/window.c\
   libdraw/writecolmap.c\
   libdraw/writeimage.c\
   libdraw/writesubfont.c\
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
   libmemdraw/openmemsubfont.c\
   libmemdraw/poly.c\
   libmemdraw/read.c\
   libmemdraw/scripts/mkcmap.c\
   libmemdraw/string.c\
   libmemdraw/subfont.c\
   libmemdraw/tests/arctest.c\
   libmemdraw/tests/memdrawtest.c\
   libmemdraw/unload.c\
   libmemdraw/write.c\
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
   ../windows/apps/clock.c\
   ../windows/apps/colors.c\
   ../windows/apps/screenlock.c\
   ../windows/libpanel/button.c\
   ../windows/libpanel/canvas.c\
   ../windows/libpanel/draw.c\
   ../windows/libpanel/edit.c\
   ../windows/libpanel/entry.c\
   ../windows/libpanel/event.c\
   ../windows/libpanel/frame.c\
   ../windows/libpanel/group.c\
   ../windows/libpanel/init.c\
   ../windows/libpanel/label.c\
   ../windows/libpanel/list.c\
   ../windows/libpanel/mem.c\
   ../windows/libpanel/message.c\
   ../windows/libpanel/pack.c\
   ../windows/libpanel/panel.h\
   ../windows/libpanel/pldefs.h\
   ../windows/libpanel/popup.c\
   ../windows/libpanel/print.c\
   ../windows/libpanel/pulldown.c\
   ../windows/libpanel/rtext.c\
   ../windows/libpanel/rtext.h\
   ../windows/libpanel/scroll.c\
   ../windows/libpanel/scrollbar.c\
   ../windows/libpanel/slider.c\
   ../windows/libpanel/snarf.c\
   ../windows/libpanel/textview.c\
   ../windows/libpanel/textwin.c\
   ../windows/libpanel/utf.c\
   ../kernel/devices/screen/386/devvga.c\
   ../kernel/devices/screen/386/screen.c\
   ../kernel/devices/screen/386/screen.h\
   ../kernel/devices/screen/386/vga.c\
   ../kernel/devices/screen/386/vgaclgd542x.c\
   ../kernel/devices/screen/386/vgavesa.c\
   ../kernel/devices/screen/386/vgax.c\
   ../kernel/devices/screen/devdraw.c\

#   ../lib_graphics/libmemdraw/hwdraw.c\
   ../kernel/devices/screen/386/cga.c\
$   codegraph.h

