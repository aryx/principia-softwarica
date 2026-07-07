/*s: init/arm/watchdog.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

/*s: constant [[POWERREGS]](arm) */
#define POWERREGS   (VIRTIO+0x100000)
/*e: constant [[POWERREGS]](arm) */

/*s: enum [[_anon_]]([[(init/arm/watchdog.c)(arm)]]) */
enum {
    Wdogfreq    = 65536,
    Wdogtime    = 5,    /* seconds, ≤ 15 */
};
/*e: enum [[_anon_]]([[(init/arm/watchdog.c)(arm)]]) */

/*s: enum [[_anon_]]([[(init/arm/watchdog.c)2(arm)]]) */
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
/*e: enum [[_anon_]]([[(init/arm/watchdog.c)2(arm)]]) */

/*s: function [[archreset]](arm) */
void
archreset(void)
{
    fpon();
}
/*e: function [[archreset]](arm) */

/*s: function [[archreboot]](arm) */
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
/*e: function [[archreboot]](arm) */

/*s: function [[wdogfeed]](arm) */
static void
wdogfeed(void)
{
    u32int *r;

    /* claude: skip under emulation. On real hardware this Rstc write refreshes a
     * 5s watchdog countdown (a safety net). QEMU's bcm2835 power-manager model
     * performs the CPU reset on the Rstc write *itself* -- it does not model the
     * countdown -- so feeding the dog every 10ms would hard-reset the machine on
     * every clock tick, and the kernel would never get past init0. We gate the
     * write rather than the registration so real hardware keeps a live watchdog
     * from the same 9pi image (see [[emulating]]). */
    if(emulating())
        return;
    r = (u32int*)POWERREGS;
    r[Wdog] = Password | (Wdogtime * Wdogfreq);
    r[Rstc] = Password | (r[Rstc] & ~CfgMask) | CfgReset;
}
/*e: function [[wdogfeed]](arm) */

/*s: function [[wdogoff]](arm) */
void
wdogoff(void)
{
    u32int *r;

    r = (u32int*)POWERREGS;
    r[Rstc] = Password | (r[Rstc] & ~CfgMask);
}
/*e: function [[wdogoff]](arm) */

/*s: function [[watchdoglink]](arm) */
void
watchdoglink(void)
{
    /* claude: register unconditionally; wdogfeed() itself no-ops under emulation
     * (QEMU resets the CPU on the Rstc write). Real hardware gets a live watchdog. */
    addclock0link(wdogfeed, Arch_HZ);
}
/*e: function [[watchdoglink]](arm) */
/*e: init/arm/watchdog.c */
