TOP=..

all:
	echo todo

include $(TOP)/docs/latex/Makefile.common

##############################################################################
# Literate Programming rules
##############################################################################

TEXMAIN=Compiler.tex.nw
TEX=Compiler.tex

# set of noweb (.nw) files

# must be in the same order of the #include for syncweb multi files support
# to work
SRC_ORIG=Compiler.tex.nw Compiler_extra.tex.nw

#TODO: cc/cc.y
SRC_VIEWS= \
   cc/cc.h\
   cc/mac.c\
   cc/omachcap.c\
   cc/globals.c\
   cc/sub.c\
   cc/bits.c\
   cc/lex.c\
   cc/compat.c\
   cc/acid.c\
   cc/pickle.c\
   cc/com64.c\
   cc/pswt.c\
   cc/dcl.c\
   cc/funct.c\
   cc/scon.c\
   cc/dpchk.c\
   cc/com.c\
   cc/pgen.c\
   8c/enam.c\
   8c/gc.h\
   8c/machcap.c\
   8c/globals2.c\
   8c/list.c\
   8c/txt.c\
   8c/peep.c\
   8c/sgen.c\
   8c/swt.c\
   8c/mul.c\
   8c/reg.c\
   8c/cgen64.c\
   8c/div.c\
   8c/cgen.c\


# y.tab.[ch]
