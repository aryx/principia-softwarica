#include <u.h>
#include <libc.h>
#include <bio.h>
#include <mach.h>
#include <tos.h>

#include "arm.h"

Registers reg;
Memory memory;
int text;
int trace;
int sysdbg;
int calltree;
//in run.c
//Inst itab[];
Icache icache;
Tlb tlb;
int count;
jmp_buf errjmp;
Breakpoint *bplist;
int atbpt;
int membpt;
int cmdcount;
int nopcount;
ulong dot;
Biobuf *bioout;
Biobuf *bin;
ulong *iprof;
Map *symmap;		

int	datasize;
ulong	textbase;
