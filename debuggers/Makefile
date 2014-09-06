TOP=..

all:
	echo todo

include $(TOP)/docs/latex/Makefile.common

##############################################################################
# Literate Programming rules
##############################################################################

TEXMAIN=Debugger.tex.nw
TEX=Debugger.tex

# set of noweb (.nw) files

# must be in the same order of the #include for syncweb multi files support
# to work
SRC_ORIG=Debugger.tex.nw Debugger_extra.tex.nw

SRC_VIEWS= \
   tracers/ktrace.c\
   tracers/strace.c\
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

