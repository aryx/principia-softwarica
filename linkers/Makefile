TOP=..

all:
	echo todo

include $(TOP)/docs/latex/Makefile.common

##############################################################################
# Literate Programming rules
##############################################################################

TEXMAIN=Linker.nw
TEX=Linker.tex

# set of noweb (.nw) files

# must be in the same order of the #include for syncweb multi files support
# to work
SRC_ORIG=Linker.nw Linker_extra.nw

# actually also dependency to 8c/enam.c for anmes
SRC_VIEWS= \
  ../include/a.out.h\
  ../include/elf.h\
  ../include/ar.h\
  ../include/bootexec.h\
  ../include/mach.h\
  misc/ar.c\
  misc/nm.c\
  misc/size.c\
  misc/strip.c\
  5l/l.h\
  5l/globals.c\
  5l/optab.c\
  5l/list.c\
  5l/compat.c\
  5l/pass.c\
  5l/asm.c\
  5l/span.c\
  5l/obj.c\
  5l/noop.c\
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
  libmach/5.c\
  libmach/5db.c\
  libmach/5obj.c\
  libmach/8.c\
  libmach/8db.c\
  libmach/8obj.c\
  libmach/elf.h\
  libmach/obj.h\
  libmach/swap.c\
  libmach/executable.c\
  libmach/map.c\
  libmach/sym.c\
  libmach/access.c\
  libmach/machdata.c\
  libmach/obj.c\
  libmach/setmach.c\
