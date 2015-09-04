TOP=..

all:
	echo todo

include $(TOP)/docs/latex/Makefile.common

##############################################################################
# Literate Programming rules
##############################################################################

TEXMAIN=Debugger.nw
TEX=Debugger.tex

# set of noweb (.nw) files

# must be in the same order of the #include for syncweb multi files support
# to work
SRC_ORIG=Debugger.nw Debugger_extra.nw Debugger_x86.nw

#TODO: acid/dbg.y cc/acid.c?
# TODO misc/leak misc/kmem? misc/umem? snap?
SRC_VIEWS= \
   tracers/ktrace.c\
   tracers/strace.c\
   \
   db/defs.h\
   db/fns.h\
   db/utils.c\
   db/globals.c\
   db/output.c\
   db/input.c\
   db/setup.c\
   db/format.c\
   db/regs.c\
   db/expr.c\
   db/trcrun.c\
   db/print.c\
   db/command.c\
   db/runpcs.c\
   db/pcs.c\
   db/main.c\
   \
   acid/acid.h\
   acid/globals.c\
   acid/lex.c\
   acid/main.c\
   acid/util.c\
   acid/exec.c\
   acid/proc.c\
   acid/list.c\
   acid/dot.c\
   acid/print.c\
   acid/expr.c\
   acid/builtin.c\
  \
  ../include/debug/mach.h\
  \
  libmach/5.c\
  libmach/5db.c\
  libmach/5obj.c\
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

XX=\
  libmach/8.c\
  libmach/8db.c\
  libmach/8obj.c\


# not acid/y.tab.[ch]

