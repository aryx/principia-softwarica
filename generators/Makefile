TOP=..

all:
	echo todo

include $(TOP)/docs/latex/Makefile.common

##############################################################################
# Literate Programming rules
##############################################################################

TEXMAIN=CompilerGenerator.nw
TEX=CompilerGenerator.tex

# set of noweb (.nw) files

# must be in the same order of the #include for syncweb multi files support
# to work
SRC_ORIG=CompilerGenerator.nw CompilerGenerator_extra.nw

#TODO yacc/yaccpar lex/ncform (which then add pragma to libl.a)   lex/parser.y
SRC_VIEWS= \
  lex/ldefs.h\
  lex/liblex/allprint.c\
  lex/liblex/main.c\
  lex/liblex/reject.c\
  lex/liblex/yyless.c\
  lex/liblex/yywrap.c\
  lex/globals.c\
  lex/header.c\
  lex/sub1.c\
  lex/sub2.c\
  lex/lmain.c\
  yacc/yacc.c\

# y.tab.[ch]

#less: can remove yaccpars, unix compatibility