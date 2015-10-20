##############################################################################
# Variables
##############################################################################

SRC=common.ml \
 ast_asm.ml \
 parser_asm.ml lexer_asm.ml parse_asm.ml \
 resolve_labels.ml object_file.ml \
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

lexer_asm.ml: lexer_asm.mll
	$(OCAMLLEX) $<
clean::
	rm -f lexer_asm.ml
beforedepend:: lexer_asm.ml


parser_asm.ml parser_asm.mli: parser_asm.mly
	$(OCAMLYACC) $<
clean::
	rm -f parser_asm.ml parser_asm.mli parser_asm.output
beforedepend:: parser_asm.ml parser_asm.mli


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
