##############################################################################
# Variables
##############################################################################

SRC=common.ml \
 ast.ml \
 parser.ml lexer.ml parse.ml \
 resolve.ml gen.ml \
 main.ml

OBJS=$(SRC:.ml=.cmo)

# Principia spirit! THE assembler! :)
PROG=assembler

all: $(PROG)

##############################################################################
# Top rules
##############################################################################

$(PROG): $(OBJS)
	$(OCAMLC) -o $@ $^

clean::
	rm -f $(PROG)

lexer.ml: lexer.mll
	$(OCAMLLEX) $<
clean::
	rm -f lexer.ml
beforedepend:: lexer.ml


parser.ml parser.mli: parser.mly
	$(OCAMLYACC) $<
clean::
	rm -f parser.ml parser.mli parser.output
beforedepend:: parser.ml parser.mli


##############################################################################
# Generic
##############################################################################

WARNING_FLAGS?=-w +A-4-29-6-45-41-44-48 -warn-error +a 
OCAMLCFLAGS=-g -thread -dtypes $(WARNING_FLAGS) $(OCAMLCFLAGS_EXTRA) 

OCAMLC=ocamlc$(OPTBIN) $(OCAMLCFLAGS) $(PP) $(INCLUDES)
OCAMLLEX=ocamllex #-ml # -ml for debugging lexer, but slightly slower
OCAMLYACC=ocamlyacc -v
OCAMLDEP=ocamldep $(PP) $(INCLUDES)
OCAMLMKTOP=ocamlmktop -g -custom $(INCLUDES) -thread

OCAMLOPT=ocamlopt$(OPTBIN) $(OPTFLAGS) $(PP) $(INCLUDES)



.SUFFIXES: .ml .mli .cmo .cmi .cmx .cmt

.ml.cmo:
	$(OCAMLC)  -c $<
.mli.cmi:
	$(OCAMLC)  -c $<
.ml.cmx:
	$(OCAMLOPT)  -c $<

clean::
	rm -f *.cm[ioxa] *.cmt* *.o *.a *.cmxa *.annot
	rm -f *~ .*~ *.exe gmon.out #*#

beforedepend::

depend:: beforedepend
	$(OCAMLDEP) *.mli *.ml > .depend

-include .depend

##############################################################################
# Devel
##############################################################################

loc: clean
	wc -l *