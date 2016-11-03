TOP=..

all:
	echo todo

include $(TOP)/docs/latex/Makefile.common

##############################################################################
# Literate Programming rules
##############################################################################

TEXMAIN=Interpreter.tex.nw
TEX=Interpreter.tex

# set of noweb (.nw) files

# must be in the same order of the #include for syncweb multi files support
# to work
SRC_ORIG=Interpreter.tex.nw Interpreter_extra.tex.nw

SRC_VIEWS= \
    s9/s9.c
