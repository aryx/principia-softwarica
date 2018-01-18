/*s: machine/5i/globals.c */
/*s: basic includes */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <mach.h>

#include "arm.h"
/*e: basic includes */

#include <tos.h>

//in run.c
//Inst itab[];

/*s: global [[reg]] */
Registers reg;
/*e: global [[reg]] */
/*s: global [[memory]] */
Memory memory;
/*e: global [[memory]] */
/*s: global [[text]] */
fdt text;
/*e: global [[text]] */
/*s: global [[trace]] */
bool trace;
/*e: global [[trace]] */
/*s: global [[sysdbg]] */
bool sysdbg;
/*e: global [[sysdbg]] */
/*s: global [[calltree]] */
bool calltree;
/*e: global [[calltree]] */
/*s: global [[icache]] */
Icache icache;
/*e: global [[icache]] */
/*s: global [[tlb]] */
Tlb tlb;
/*e: global [[tlb]] */
/*s: global [[count]] */
int count;
/*e: global [[count]] */
/*s: global [[errjmp]] */
jmp_buf errjmp;
/*e: global [[errjmp]] */
/*s: global [[bplist]] */
// list<Breakpoint> (next = Breakpoint.next)
Breakpoint *bplist;
/*e: global [[bplist]] */
/*s: global [[atbpt]] */
bool atbpt;
/*e: global [[atbpt]] */
/*s: global [[membpt]] */
bool membpt;
/*e: global [[membpt]] */
/*s: global [[cmdcount]] */
int cmdcount;
/*e: global [[cmdcount]] */
/*s: global [[nopcount]] */
int nopcount;
/*e: global [[nopcount]] */
/*s: global [[dot]] */
uintptr dot;
/*e: global [[dot]] */
/*s: global [[bixxx]] */
Biobuf *bin, *bout;
/*e: global [[bixxx]] */
/*s: global [[iprof]] */
ulong *iprof;
/*e: global [[iprof]] */
/*s: global [[symmap]] */
Map *symmap;		
/*e: global [[symmap]] */
/*s: global [[datasize]] */
int	datasize;
/*e: global [[datasize]] */
/*s: global [[textbase]] */
uintptr	textbase;
/*e: global [[textbase]] */
/*e: machine/5i/globals.c */
