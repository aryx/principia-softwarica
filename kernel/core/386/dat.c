/*s: core/386/dat.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

/*s: global cpu(x86) */
// ref<Cpu>, assigned to CPUADDR in _clearbss
Cpu *cpu;
/*e: global cpu(x86) */
/*e: core/386/dat.c */
