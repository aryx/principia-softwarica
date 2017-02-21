/*s: init/arm/raspi1.c */
/*
 * bcm2835 (e.g. original raspberry pi) architecture-specific stuff
 */
#include "u.h"
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

#include "io.h"
#include "arm.h"

#include "../port/netif.h"
#include "../port/etherif.h"

/*s: global soc(arm) */
Soc soc = {
    .dramsize	= 512*MiB,    // 0 to 0x20000000

    .physio		= 0x20000000, // after RAM
    .busdram	= 0x40000000,
    .busio		= 0x7E000000,
    .armlocal	= 0,

    .l1ptedramattrs = Cached | Buffered,
    .l2ptedramattrs = Cached | Buffered,
};
/*e: global soc(arm) */

    
/*s: function cputype2name(arm) */
char*
cputype2name(char *buf, int size)
{
    seprint(buf, buf + size, "1176JZF-S");
    return buf;
}
/*e: function cputype2name(arm) */

/*s: function arch_cpuidprint(arm) */
void
arch_cpuidprint(void)
{
    char name[64];

    cputype2name(name, sizeof name);
    arch_delay(50);				/* let uart catch up */
    print("cpu%d: %dMHz ARM %s\n", cpu->cpuno, cpu->cpumhz, name);
}
/*e: function arch_cpuidprint(arm) */

/*s: function getncpus(arm) */
int
getncpus(void)
{
    return 1;
}
/*e: function getncpus(arm) */

/*s: function startcpus(arm) */
int
startcpus(uint)
{
    return 1;
}
/*e: function startcpus(arm) */


/*s: function archether(arm) */
int
archether(unsigned ctlrno, Ether *ether)
{
    ether->type = "usb";
    ether->ctlrno = ctlrno;
    ether->irq = -1;
    ether->nopt = 0;
    return 1;
}
/*e: function archether(arm) */

/*s: function l2ap(arm) */
int
l2ap(int ap)
{
    return (AP(3, (ap))|AP(2, (ap))|AP(1, (ap))|AP(0, (ap)));
}
/*e: function l2ap(arm) */
/*e: init/arm/raspi1.c */
