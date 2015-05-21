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

sync::
	$(MAKE) sync_ml

CAML=/home/pad/github/fork-ocaml

SRC_VIEWS_ML=\
  $(CAML)/lex/ast.ml \
  $(CAML)/lex/output.mli \
  $(CAML)/lex/output.ml \
  $(CAML)/lex/lexgen.mli \
  $(CAML)/lex/lexgen.ml \
  $(CAML)/lex/compact.mli \
  $(CAML)/lex/compact.ml \
  $(CAML)/lex/main.ml \
  $(CAML)/lex/lexer.mll \
  $(CAML)/stdlib/lexing.mli \
  $(CAML)/stdlib/lexing.ml \


SYNCWEB_ML=~/github/syncweb/syncweb -md5sum_in_auxfile -less_marks -lang ocaml

sync_ml:
	for i in $(SRC_VIEWS_ML); do echo $$i; $(SYNCWEB_ML) $(SRC_ORIG) $$i || exit 1; done


sync::
	$(MAKE) sync_yacc

SYNCWEB_YACC=~/github/syncweb/syncweb -md5sum_in_auxfile -less_marks -lang ocamlyacc
sync_yacc:
	$(SYNCWEB_YACC) $(SRC_ORIG) $(CAML)/lex/parser.mly

