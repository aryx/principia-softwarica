TOP=..

all:
	echo todo

include $(TOP)/docs/latex/Makefile.common

##############################################################################
# Literate Programming rules
##############################################################################

DOC=Linker

# set of noweb (.nw) files

# must be in the same order of the #include for syncweb multi files support
# to work
SRC_ORIG=Linker.nw Linker_extra.nw Linker_x86.nw

# actually also dependency to 8c/enam.c and 5c/enam.c for anames
SRC_VIEWS= \
  ../include/exec/a.out.h\
  ../include/exec/elf.h\
  ../include/devel/output/ar.h\
  ../include/devel/output/bootexec.h\
  misc/ar.c\
  misc/nm.c\
  misc/size.c\
  misc/strip.c\
  5l/l.h\
  5l/globals.c\
  5l/optab.c\
  5l/utils.c\
  5l/list.c\
  5l/compat.c\
  5l/pass.c\
  5l/asm.c\
  5l/span.c\
  5l/obj.c\
  5l/noop.c\

XX=\
  8l/elf.h\
  8l/elf.c\
  8l/l.h\
  8l/globals.c\
  8l/utils.c\
  8l/optab.c\
  8l/list.c\
  8l/compat.c\
  8l/pass.c\
  8l/asm.c\
  8l/span.c\
  8l/obj.c\
