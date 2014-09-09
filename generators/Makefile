TOP=..

all:
	echo todo

include $(TOP)/docs/latex/Makefile.common

##############################################################################
# Literate Programming rules
##############################################################################

TEXMAIN=CompilerGenerator.tex.nw
TEX=CompilerGenerator.tex

# set of noweb (.nw) files

# must be in the same order of the #include for syncweb multi files support
# to work
SRC_ORIG=CompilerGenerator.tex.nw CompilerGenerator_extra.tex.nw

#TODO yacc/yaccpar lex/ncform (which then add pragma to libl.a)   lex/parser.y
SRC_VIEWS= \


#less: can remove yaccpars, unix compatibility