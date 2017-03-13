/*s: init/arm/watchdog.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

/*s: constant POWERREGS(arm) */
#define POWERREGS   (VIRTIO+0x100000)
/*e: constant POWERREGS(arm) */

/*s: enum _anon_ (init/arm/watchdog.c)(arm) */
enum {
    Wdogfreq    = 65536,
    Wdogtime    = 5,    /* seconds, ≤ 15 */
};
/*e: enum _anon_ (init/arm/watchdog.c)(arm) */

/*s: enum _anon_ (init/arm/watchdog.c)2(arm) */
/*
 * Power management / watchdog registers
 */
enum {
    Rstc        = 0x1c>>2,
        Password    = 0x5A<<24,
        CfgMask     = 0x03<<4,
        CfgReset    = 0x02<<4,
    Rsts        = 0x20>>2,
    Wdog        = 0x24>>2,
};
/*e: enum _anon_ (init/arm/watchdog.c)2(arm) */

/*s: function archreset(arm) */
void
archreset(void)
{
    fpon();
}
/*e: function archreset(arm) */

/*s: function archreboot(arm) */
void
archreboot(void)
{
    u32int *r;

    r = (u32int*)POWERREGS;
    r[Wdog] = Password | 1;
    r[Rstc] = Password | (r[Rstc] & ~CfgMask) | CfgReset;
    arch_coherence();
    for(;;)
        ;
}
/*e: function archreboot(arm) */

/*s: function wdogfeed(arm) */
static void
wdogfeed(void)
{
    u32int *r;

    r = (u32int*)POWERREGS;
    r[Wdog] = Password | (Wdogtime * Wdogfreq);
    r[Rstc] = Password | (r[Rstc] & ~CfgMask) | CfgReset;
}
/*e: function wdogfeed(arm) */

/*s: function wdogoff(arm) */
void
wdogoff(void)
{
    u32int *r;

    r = (u32int*)POWERREGS;
    r[Rstc] = Password | (r[Rstc] & ~CfgMask);
}
/*e: function wdogoff(arm) */

/*s: function watchdoglink(arm) */
void
watchdoglink(void)
{
    addclock0link(wdogfeed, Arch_HZ);
}
/*e: function watchdoglink(arm) */
/*e: init/arm/watchdog.c */
