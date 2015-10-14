TOP=..

all:
	echo todo

include $(TOP)/docs/latex/Makefile.common

##############################################################################
# Literate Programming rules
##############################################################################

DOC=Assembler

# set of noweb (.nw) files
# must be in the same order of the #include for syncweb multi files support
# to work
SRC_ORIG=Assembler.nw Assembler_extra.nw Assembler_x86.nw

SRC_VIEWS= \
  aa/aa.h\
  aa/globals.c\
  aa/compat.c\
  aa/utils.c\
  aa/lexbody.c\
  aa/macbody.c\
  \
  ../include/devel/output/common.out.h\
  ../include/arch/arm/5.out.h\
  \
  5a/a.h\
  5a/globals.c\
  5a/lex.c\
  5a/a.y\
  5a/main.c\
  \
  ../lib_core/libc/arm/div.s


# temporarily split from SRC_VIEWS for 'make loc'
XX=\
  ../include/arch/386/8.out.h\
  misc/data2s.c\
  8a/a.h\
  8a/globals.c\
  8a/lex.c\
  8a/a.y \

#skip: 8a/y.tab.[ch], comes from 8a/a.y
