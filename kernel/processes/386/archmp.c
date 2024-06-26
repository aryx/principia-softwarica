/*s: processes/386/archmp.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */
#include "io.h"
#include "mp.h"

_MP_ *_mp_;

/*s: archmp.c forward decl(x86) */
// forward decl, mutual recursivity between archmp and identity
extern PCArch archmp;
/*e: archmp.c forward decl(x86) */


uvlong
tscticks(uvlong *hz)
{
    if(hz != nil)
        *hz = cpu->cpuhz;

    arch_cycles(&cpu->tscticks);   /* Uses the rdtsc instruction */
    return cpu->tscticks;
}

static void
mpresetothers(void)
{
    /*
     * INIT all excluding self.
     */
    lapicicrw(0, 0x000C0000|ApicINIT);
}

int
identify(void)
{
    char *cp;
    PCMP *pcmp;
    uchar *p, sum;
    ulong length;

    if((cp = getconf("*nomp")) != nil && strtol(cp, 0, 0) != 0)
        return 1;

    /*
     * Search for an MP configuration table. For now,
     * don't accept the default configurations (physaddr == 0).
     * Check for correct signature, calculate the checksum and,
     * if correct, check the version.
     * To do: check extended table checksum.
     */
    if((_mp_ = sigsearch("_MP_")) == 0 || _mp_->physaddr == 0) {
        /*
         * we can easily get processor info from acpi, but
         * interrupt routing, etc. would require interpreting aml.
         */
        print("archmp: no mp table found, assuming uniprocessor\n");
        return 1;
    }

    if (0)
        iprint("mp physaddr %#lux\n", _mp_->physaddr);
    pcmp = KADDR(_mp_->physaddr);
    if(memcmp(pcmp, "PCMP", 4) != 0) {
        print("archmp: mp table has bad magic");
        return 1;
    }

    length = pcmp->length;
    sum = 0;
    for(p = (uchar*)pcmp; length; length--)
        sum += *p++;

    if(sum || (pcmp->version != 1 && pcmp->version != 4))
        return 1;

    if(cpuserver && cpu->havetsc)
        archmp.fastclock = tscticks;
    return 0;
}

/*s: global [[archmp]](x86) */
PCArch archmp = {
    .id=        "_MP_", 
    .ident=     identify,
    .reset=     mpshutdown,
    .intrinit=  mpinit,
    .intrenable=    mpintrenable,
    .intron=    lapicintron,
    .introff=   lapicintroff,
    .fastclock= i8253read,
    .timerset=  lapictimerset,
    .resetothers=   mpresetothers,
};
/*e: global [[archmp]](x86) */


//Lock mpsynclock;

/*s: function [[syncclock]](x86) */
// actually not a clock callback even though finish in clock
void
syncclock(void)
{
    uvlong x;

    if(arch->fastclock != tscticks)
        return;

    if(cpu->cpuno == 0){
        wrmsr(0x10, 0);
        cpu->tscticks = 0;
    } else {
        x = CPUS(0)->tscticks;
        while(x == CPUS(0)->tscticks)
            ;
        wrmsr(0x10, CPUS(0)->tscticks);
        arch_cycles(&cpu->tscticks);
    }
}
/*e: function [[syncclock]](x86) */
/*e: processes/386/archmp.c */
