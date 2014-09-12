TOP=..

all:
	echo todo

include $(TOP)/docs/latex/Makefile.common

##############################################################################
# Literate Programming rules
##############################################################################

TEXMAIN=Assembler.tex.nw
TEX=Assembler.tex

# set of noweb (.nw) files

# must be in the same order of the #include for syncweb multi files support
# to work
SRC_ORIG=Assembler.tex.nw Assembler_extra.tex.nw

SRC_VIEWS= \
  ../include/386/8.out.h\
  ../include/common.out.h\
  misc/data2s.c\
  aa/aa.h\
  aa/compat.c\
  aa/globals.c\
  aa/lexbody.c\
  aa/macbody.c\
  8a/a.h\
  8a/globals.c\
  8a/lex.c\
  8a/a.y

#skip: 8a/y.tab.[ch], comes from 8a/a.y
