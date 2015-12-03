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
  ../include/obj/common.out.h\
  ../include/obj/5.out.h\
  \
  5a/a.h\
  5a/globals.c\
  5a/lex.c\
  5a/a.y\
  5a/main.c\
  \
  ../lib_core/libc/arm/div.s
#TODO: add 5a/tests/helloworld.s at some point and many more .s


# temporarily split from SRC_VIEWS for 'make loc'
XX=\
  ../include/obj/8.out.h\
  misc/data2s.c\
  8a/a.h\
  8a/globals.c\
  8a/lex.c\
  8a/a.y \

#skip: 8a/y.tab.[ch], comes from 8a/a.y
