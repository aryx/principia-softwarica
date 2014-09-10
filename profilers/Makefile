TOP=..

all:
	echo todo

include $(TOP)/docs/latex/Makefile.common

##############################################################################
# Literate Programming rules
##############################################################################

TEXMAIN=Profiler.tex.nw
TEX=Profiler.tex

# set of noweb (.nw) files

# must be in the same order of the #include for syncweb multi files support
# to work
SRC_ORIG=Profiler.tex.nw Profiler_extra.tex.nw


#lib_core/libc/profile.c!! tos.h!! trace.h
SRC_VIEWS= \
  time.c\
  kprof.c\
  prof.c\
  trace.c\
  tprof.c\
  stats.c\
  iostats/statfs.h\
  iostats/globals.c\
  iostats/iostats.c\
  iostats/statsrv.c\
