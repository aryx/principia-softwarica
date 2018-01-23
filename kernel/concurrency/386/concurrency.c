/*s: concurrency/386/concurrency.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

/*s: function cmpswap and default implementation(x86) */
/*
 * 386 has no compare-and-swap instruction.
 * Run it with interrupts turned off instead.
 */
// used to be static, but now shared between arch.c and devarch.c
int
cmpswap386(long *addr, long old, long new)
{
    int r, s;

    s = arch_splhi();
    if(r = (*addr == old))
        *addr = new;
    arch_splx(s);
    return r;
}

int (*arch_cmpswap)(long*, long, long) = cmpswap386;
/*e: function cmpswap and default implementation(x86) */

/*
 * On a uniprocessor, you'd think that coherence could be nop,
 * but it can't.  We still need a barrier when using coherence() in
 * device drivers.
 *
 * On VMware, it's safe (and a huge win) to set this to nop.
 * Aux/vmware does this via the #P/archctl file.
 */
//now in globals.c: void (*coherence)(void) = nop;
/*e: concurrency/386/concurrency.c */
