/*s: arch/386/arch.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

/*s: global arch(x86) */
PCArch* arch;
/*e: global arch(x86) */

/*s: hook fprestore and fpsave(x86) */
void    (*fprestore)(Arch_FPsave*);
void    (*fpsave)(Arch_FPsave*);
/*e: hook fprestore and fpsave(x86) */

int (*_pcmspecial)(char*, ISAConf*);
//void (*_pcmspecialclose)(int); // useful if use pccard device driver
/*
 *  call either the pcmcia or pccard device setup
 */
int
pcmspecial(char *idstr, ISAConf *isa)
{
    return (_pcmspecial != nil)? _pcmspecial(idstr, isa): -1;
}

/*s: function cpuidprint(x86) */
void
arch_cpuidprint(void)
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
/*e: function cpuidprint(x86) */

/*e: arch/386/arch.c */
