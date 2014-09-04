
all:
	echo todo

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

TEXMAIN=Windows.tex.nw
TEX=Windows.tex

# set of noweb (.nw) files

# must be in the same order of the #include for syncweb multi files support
# to work
SRC_ORIG=Windows.tex.nw Windows_extra.tex.nw

# from list_windows
SRC_VIEWS= \
   ../include/complete.h\
   ../include/cursor.h\
   ../include/draw.h\
   ../include/event.h\
   ../include/frame.h\
   ../include/keyboard.h\
   ../include/memdraw.h\
   ../include/memlayer.h\
   ../include/mouse.h\
   ../include/plumb.h\
   ../lib_graphics/libdraw/alloc.c\
   ../lib_graphics/libdraw/allocimagemix.c\
   ../lib_graphics/libdraw/arith.c\
   ../lib_graphics/libdraw/bezier.c\
   ../lib_graphics/libdraw/border.c\
   ../lib_graphics/libdraw/buildfont.c\
   ../lib_graphics/libdraw/bytesperline.c\
   ../lib_graphics/libdraw/chan.c\
   ../lib_graphics/libdraw/cloadimage.c\
   ../lib_graphics/libdraw/computil.c\
   ../lib_graphics/libdraw/creadimage.c\
   ../lib_graphics/libdraw/debug.c\
   ../lib_graphics/libdraw/defont.c\
   ../lib_graphics/libdraw/draw.c\
   ../lib_graphics/libdraw/drawrepl.c\
   ../lib_graphics/libdraw/egetrect.c\
   ../lib_graphics/libdraw/ellipse.c\
   ../lib_graphics/libdraw/emenuhit.c\
   ../lib_graphics/libdraw/event.c\
   ../lib_graphics/libdraw/fmt.c\
   ../lib_graphics/libdraw/font.c\
   ../lib_graphics/libdraw/freesubfont.c\
   ../lib_graphics/libdraw/getdefont.c\
   ../lib_graphics/libdraw/getrect.c\
   ../lib_graphics/libdraw/getsubfont.c\
   ../lib_graphics/libdraw/icossin.c\
   ../lib_graphics/libdraw/icossin2.c\
   ../lib_graphics/libdraw/init.c\
   ../lib_graphics/libdraw/keyboard.c\
   ../lib_graphics/libdraw/line.c\
   ../lib_graphics/libdraw/loadimage.c\
   ../lib_graphics/libdraw/menuhit.c\
   ../lib_graphics/libdraw/mkfont.c\
   ../lib_graphics/libdraw/mouse.c\
   ../lib_graphics/libdraw/newwindow.c\
   ../lib_graphics/libdraw/openfont.c\
   ../lib_graphics/libdraw/poly.c\
   ../lib_graphics/libdraw/readcolmap.c\
   ../lib_graphics/libdraw/readimage.c\
   ../lib_graphics/libdraw/readsubfont.c\
   ../lib_graphics/libdraw/rectclip.c\
   ../lib_graphics/libdraw/replclipr.c\
   ../lib_graphics/libdraw/rgb.c\
   ../lib_graphics/libdraw/scroll.c\
   ../lib_graphics/libdraw/string.c\
   ../lib_graphics/libdraw/stringbg.c\
   ../lib_graphics/libdraw/stringsubfont.c\
   ../lib_graphics/libdraw/stringwidth.c\
   ../lib_graphics/libdraw/subfont.c\
   ../lib_graphics/libdraw/subfontcache.c\
   ../lib_graphics/libdraw/subfontname.c\
   ../lib_graphics/libdraw/tests/drawtest.c\
   ../lib_graphics/libdraw/unloadimage.c\
   ../lib_graphics/libdraw/window.c\
   ../lib_graphics/libdraw/writecolmap.c\
   ../lib_graphics/libdraw/writeimage.c\
   ../lib_graphics/libdraw/writesubfont.c\
   ../lib_graphics/libmemdraw/alloc.c\
   ../lib_graphics/libmemdraw/arc.c\
   ../lib_graphics/libmemdraw/cload.c\
   ../lib_graphics/libmemdraw/cmap.c\
   ../lib_graphics/libmemdraw/cread.c\
   ../lib_graphics/libmemdraw/defont.c\
   ../lib_graphics/libmemdraw/draw.c\
   ../lib_graphics/libmemdraw/ellipse.c\
   ../lib_graphics/libmemdraw/fillpoly.c\
   ../lib_graphics/libmemdraw/hwdraw.c\
   ../lib_graphics/libmemdraw/iprint.c\
   ../lib_graphics/libmemdraw/line.c\
   ../lib_graphics/libmemdraw/load.c\
   ../lib_graphics/libmemdraw/openmemsubfont.c\
   ../lib_graphics/libmemdraw/poly.c\
   ../lib_graphics/libmemdraw/read.c\
   ../lib_graphics/libmemdraw/scripts/mkcmap.c\
   ../lib_graphics/libmemdraw/string.c\
   ../lib_graphics/libmemdraw/subfont.c\
   ../lib_graphics/libmemdraw/tests/arctest.c\
   ../lib_graphics/libmemdraw/tests/memdrawtest.c\
   ../lib_graphics/libmemdraw/unload.c\
   ../lib_graphics/libmemdraw/write.c\
   ../lib_graphics/libmemlayer/draw.c\
   ../lib_graphics/libmemlayer/lalloc.c\
   ../lib_graphics/libmemlayer/layerop.c\
   ../lib_graphics/libmemlayer/ldelete.c\
   ../lib_graphics/libmemlayer/lhide.c\
   ../lib_graphics/libmemlayer/line.c\
   ../lib_graphics/libmemlayer/load.c\
   ../lib_graphics/libmemlayer/lorigin.c\
   ../lib_graphics/libmemlayer/lsetrefresh.c\
   ../lib_graphics/libmemlayer/ltofront.c\
   ../lib_graphics/libmemlayer/ltorear.c\
   ../lib_graphics/libmemlayer/unload.c\
   rio/dat.h\
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
   apps/clock.c\
   apps/colors.c\
   apps/lens.c\
   apps/screenlock.c\
   apps/statusbar.c\
   apps/winwatch.c\
   codegraph.h\
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
   libpanel/button.c\
   libpanel/canvas.c\
   libpanel/draw.c\
   libpanel/edit.c\
   libpanel/entry.c\
   libpanel/event.c\
   libpanel/frame.c\
   libpanel/group.c\
   libpanel/init.c\
   libpanel/label.c\
   libpanel/list.c\
   libpanel/mem.c\
   libpanel/message.c\
   libpanel/pack.c\
   libpanel/panel.h\
   libpanel/pldefs.h\
   libpanel/popup.c\
   libpanel/print.c\
   libpanel/pulldown.c\
   libpanel/rtext.c\
   libpanel/rtext.h\
   libpanel/scroll.c\
   libpanel/scrollbar.c\
   libpanel/slider.c\
   libpanel/snarf.c\
   libpanel/textview.c\
   libpanel/textwin.c\
   libpanel/utf.c\
   libplumb/event.c\
   libplumb/mesg.c\
   libplumb/plumbsendtext.c\



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
