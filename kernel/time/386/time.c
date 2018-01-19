/*s: time/386/time.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

/*s: function cycles and default implementation(x86) */
static void
simplecycles(uvlong *x)
{
    *x = cpu->ticks;
}

void    (*arch_cycles)(uvlong*) = simplecycles;
/*e: function cycles and default implementation(x86) */

/*s: function [[timerset]](x86) */
// used to be static, but now shared between arch.c and devarch.c
int doi8253set = 1;
/*
 *  set next timer interrupt
 */
void
arch_timerset(Tval x)
{
    if(doi8253set)
        (*arch->timerset)(x);
}
/*e: function [[timerset]](x86) */

/*s: function [[us]](x86) */
ulong
arch_us(void)
{
    return fastticks2us((*arch->fastclock)(nil));
}
/*e: function [[us]](x86) */

/*s: function [[fastticks]](x86) */
/*
 *  return value and speed of timer set in arch->clockenable
 */
uvlong
devarch_arch_fastticks(uvlong *hz)
{
    return (*arch->fastclock)(hz);
}
/*e: function [[fastticks]](x86) */
/*e: time/386/time.c */
