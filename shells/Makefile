TOP=..

all:
	echo todo

include $(TOP)/docs/latex/Makefile.common

##############################################################################
# Literate Programming rules
##############################################################################

TEXMAIN=Shell.tex.nw
TEX=Shell.tex

# set of noweb (.nw) files

# must be in the same order of the #include for syncweb multi files support
# to work
SRC_ORIG=Shell.tex.nw Shell_extra.tex.nw

#rc/x.tab.h

SRC_VIEWS= \
  misc/echo.c\
  misc/pwd.c\
  misc/test.c\
  sh/sh.c\
  rc/fns.h\
  rc/getflags.h\
  rc/io.h\
  rc/syn.y\
  rc/rc.h\
  rc/exec.h\
  rc/globals.c\
  rc/getflags.c\
  rc/io.c\
  rc/var.c\
  rc/subr.c\
  rc/plan9.c\
  rc/glob.c\
  rc/exec.c\
  rc/havefork.c\
  rc/tree.c\
  rc/lex.c\
  rc/trap.c\
  rc/simple.c\
  rc/pcmd.c\
  rc/here.c\
  rc/code.c\
  rc/pfnc.c\
