TOP=..

all:
	echo todo

include $(TOP)/docs/latex/Makefile.common

##############################################################################
# Literate Programming rules
##############################################################################

TEXMAIN=Machine.tex.nw
TEX=Machine.tex

# set of noweb (.nw) files

# must be in the same order of the #include for syncweb multi files support
# to work
SRC_ORIG=Machine.tex.nw Machine_extra.tex.nw

SRC_VIEWS= \
 5i/arm.h\
 5i/icache.c\
 5i/globals.c\
 5i/utils.c\
 5i/bpt.c\
 5i/mem.c\
 5i/symbols.c\
 5i/run.c\
 5i/5i.c\
 5i/stats.c\
 5i/cmd.c\
 5i/syscall.c\
