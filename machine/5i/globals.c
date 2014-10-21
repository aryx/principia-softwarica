/*s: machine/5i/globals.c */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <mach.h>

#include <tos.h>

#include "arm.h"

/*s: global reg */
Registers reg;
/*e: global reg */
/*s: global memory */
Memory memory;
/*e: global memory */
/*s: global text */
int text;
/*e: global text */
/*s: global trace */
bool trace;
/*e: global trace */
/*s: global sysdbg */
bool sysdbg;
/*e: global sysdbg */
/*s: global calltree */
bool calltree;
/*e: global calltree */
//in run.c
//Inst itab[];
/*s: global icache */
Icache icache;
/*e: global icache */
/*s: global tlb */
Tlb tlb;
/*e: global tlb */
/*s: global count */
int count;
/*e: global count */
/*s: global errjmp */
jmp_buf errjmp;
/*e: global errjmp */
/*s: global bplist */
Breakpoint *bplist;
/*e: global bplist */
/*s: global atbpt */
int atbpt;
/*e: global atbpt */
/*s: global membpt */
int membpt;
/*e: global membpt */
/*s: global cmdcount */
int cmdcount;
/*e: global cmdcount */
/*s: global nopcount */
int nopcount;
/*e: global nopcount */
/*s: global dot */
ulong dot;
/*e: global dot */
/*s: global bioout */
Biobuf *bioout;
/*e: global bioout */
/*s: global bin */
Biobuf *bin;
/*e: global bin */
/*s: global iprof */
ulong *iprof;
/*e: global iprof */
/*s: global symmap */
Map *symmap;		
/*e: global symmap */

/*s: global datasize */
int	datasize;
/*e: global datasize */
/*s: global textbase */
ulong	textbase;
/*e: global textbase */
/*e: machine/5i/globals.c */
