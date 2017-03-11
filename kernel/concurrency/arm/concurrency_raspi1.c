/*s: concurrency/arm/concurrency_raspi1.c */
/*
 * atomic ops
 * make sure that we don't drag in the C library versions
 */
#include "u.h"
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

/*s: function arch_xdec(arm) */
long
arch_xdec(long *p)
{
    int s, v;

    s = arch_splhi();
    v = --*p;
    arch_splx(s);
    return v;
}
/*e: function arch_xdec(arm) */

/*s: function arch_xinc(arm) */
void
arch_xinc(long *p)
{
    int s;

    s = arch_splhi();
    ++*p;
    arch_splx(s);
}
/*e: function arch_xinc(arm) */

/*s: function ainc(arm) */
// overwrite libc!
int
ainc(int *p)
{
    int s, v;

    s = arch_splhi();
    v = ++*p;
    arch_splx(s);
    return v;
}
/*e: function ainc(arm) */

/*s: function adec(arm) */
int
adec(int *p)
{
    int s, v;

    s = arch_splhi();
    v = --*p;
    arch_splx(s);
    return v;
}
/*e: function adec(arm) */

/*s: function cas32(arm) */
bool
cas32(void* addr, u32int old, u32int new)
{
    bool r;
    int s;

    s = arch_splhi();
    r = (*(u32int*)addr == old);
    if(r)
        *(u32int*)addr = new;
    arch_splx(s);
    if (r)
        arch_coherence();
    return r;
}
/*e: function cas32(arm) */

/*s: function arch_cmpswap(arm) */
bool
arch_cmpswap(long *addr, long old, long new)
{
    return cas32(addr, old, new);
}
/*e: function arch_cmpswap(arm) */
/*e: concurrency/arm/concurrency_raspi1.c */
