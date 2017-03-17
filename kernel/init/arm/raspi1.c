/*s: init/arm/raspi1.c */
/*
 * bcm2835 (e.g. original raspberry pi) architecture-specific stuff
 */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

#include "io.h"
#include "arm.h"

#include "../port/netif.h"
#include "../port/etherif.h"

/*s: global soc(raspberry pi1)(arm) */
Soc soc = {
    .dramsize   = 512*MiB,    // 0 to 0x20000000
    .physio     = 0x20000000, // after RAM
    /*s: [[soc(raspberry pi1)]] other fields(arm) */
    .busdram    = 0x40000000,
    .busio      = 0x7E000000,
    /*x: [[soc(raspberry pi1)]] other fields(arm) */
    .l1ptedramattrs = Cached | Buffered,
    .l2ptedramattrs = Cached | Buffered,
    /*x: [[soc(raspberry pi1)]] other fields(arm) */
    .armlocal   = 0,
    /*e: [[soc(raspberry pi1)]] other fields(arm) */
};
/*e: global soc(raspberry pi1)(arm) */

    
/*s: function cputype2name(raspberry pi1)(arm) */
char*
cputype2name(char *buf, int size)
{
    seprint(buf, buf + size, "1176JZF-S");
    return buf;
}
/*e: function cputype2name(raspberry pi1)(arm) */

/*s: function arch_cpuidprint(raspberry pi1)(arm) */
void
arch_cpuidprint(void)
{
    char name[64];

    cputype2name(name, sizeof name);
    arch_delay(50);             /* let uart catch up */
    print("cpu%d: %dMHz ARM %s\n", cpu->cpuno, cpu->cpumhz, name);
}
/*e: function arch_cpuidprint(raspberry pi1)(arm) */

/*s: function getncpus(raspberry pi1)(arm) */
int
getncpus(void)
{
    return 1;
}
/*e: function getncpus(raspberry pi1)(arm) */

/*s: function startcpus(raspberry pi1)(arm) */
int
startcpus(uint)
{
    return 1;
}
/*e: function startcpus(raspberry pi1)(arm) */


/*s: function archether(raspberry pi1)(arm) */
int
archether(unsigned ctlrno, Ether *ether)
{
    ether->type = "usb";
    ether->ctlrno = ctlrno;
    ether->irq = -1;
    ether->nopt = 0;
    return 1;
}
/*e: function archether(raspberry pi1)(arm) */

/*s: function l2ap(raspberry pi1)(arm) */
int
l2ap(int ap)
{
    return (AP(3, (ap))|AP(2, (ap))|AP(1, (ap))|AP(0, (ap)));
}
/*e: function l2ap(raspberry pi1)(arm) */
/*e: init/arm/raspi1.c */
