/*s: arch.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */
//todo: weird, if don't include this file, then can't declare arch in this file
// or I get some type signature mismatch
#include "io.h"

/*s: global arch */
PCArch* arch;
/*e: global arch */

/*s: hook fprestore and fpsave */
void    (*fprestore)(ArchFPsave*);
void    (*fpsave)(ArchFPsave*);
/*e: hook fprestore and fpsave */

int (*_pcmspecial)(char*, ISAConf*);
void (*_pcmspecialclose)(int);

/*
 *  call either the pcmcia or pccard device setup
 */
int
pcmspecial(char *idstr, ISAConf *isa)
{
    return (_pcmspecial != nil)? _pcmspecial(idstr, isa): -1;
}

/*s: function cpuidprint */
void
cpuidprint(void)
{
    int i;
    char buf[128];

    i = snprint(buf, sizeof buf, "cpu%d: %s%dMHz ", cpu->cpuno,
        cpu->cpuno < 10? " ": "", cpu->cpumhz);
    if(cpu->cpuidid[0])
        i += sprint(buf+i, "%12.12s ", cpu->cpuidid);
    seprint(buf+i, buf + sizeof buf - 1,
        "%s (cpuid: AX 0x%4.4uX DX 0x%4.4uX)\n",
        cpu->cpuidtype, cpu->cpuidax, cpu->cpuiddx);
    print(buf);
}
/*e: function cpuidprint */

/*s: function cycles and default implementation */
static void
simplecycles(uvlong *x)
{
    *x = cpu->ticks;
}

void    (*cycles)(uvlong*) = simplecycles;
/*e: function cycles and default implementation */

/*s: function cmpswap and default implementation */
/*
 * 386 has no compare-and-swap instruction.
 * Run it with interrupts turned off instead.
 */
// used to be static, but now shared between arch.c and devarch.c
int
cmpswap386(long *addr, long old, long new)
{
    int r, s;

    s = splhi();
    if(r = (*addr == old))
        *addr = new;
    splx(s);
    return r;
}

int (*cmpswap)(long*, long, long) = cmpswap386;
/*e: function cmpswap and default implementation */

/*
 * On a uniprocessor, you'd think that coherence could be nop,
 * but it can't.  We still need a barrier when using coherence() in
 * device drivers.
 *
 * On VMware, it's safe (and a huge win) to set this to nop.
 * Aux/vmware does this via the #P/archctl file.
 */
//now in globals.c: void (*coherence)(void) = nop;


/*s: function timerset */
// used to be static, but now shared between arch.c and devarch.c
int doi8253set = 1;
/*
 *  set next timer interrupt
 */
void
timerset(Tval x)
{
    if(doi8253set)
        (*arch->timerset)(x);
}
/*e: function timerset */

/*s: function us */
ulong
us(void)
{
    return fastticks2us((*arch->fastclock)(nil));
}
/*e: function us */

/*s: function fastticks */
/*
 *  return value and speed of timer set in arch->clockenable
 */
uvlong
devarch_fastticks(uvlong *hz)
{
    return (*arch->fastclock)(hz);
}
/*e: function fastticks */
/*e: arch.c */
