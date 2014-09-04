
all:
	echo todo

##############################################################################
# Generic Literate programming variables
##############################################################################

SYNCFLAGS=-md5sum_in_auxfile -less_marks -lang C

SYNCWEB=~/github/syncweb/syncweb.opt $(SYNCFLAGS)
NOWEB=~/github/syncweb/scripts/noweblatex
PDFLATEX=pdflatex --shell-escape 

lpclean::
	rm -f *.aux *.toc *.log *.brf *.out

##############################################################################
# Literate Programming rules
##############################################################################

TEXMAIN=Windows.tex.nw
TEX=Windows.tex

# set of noweb (.nw) files

# must be in the same order of the #include for syncweb multi files support
# to work
SRC_ORIG=Windows.tex.nw Windows_extra.tex.nw

SRC_VIEWS= \



sync:
	for i in $(SRC_VIEWS); do echo $$i; $(SYNCWEB) $(SRC_ORIG) $$i || exit 1; done 

pdf:
	$(NOWEB) $(TEXMAIN) > $(TEX)
	pdflatex $(TEX)
	pdflatex $(TEX)

lpclean::
	rm -f $(TEX)

lpdistclean::
	rm -f $(SRC_VIEWS) .md5sum_* $(TEX)

clean::
	rm -f *.aux *.toc *.log *.brf *.out
