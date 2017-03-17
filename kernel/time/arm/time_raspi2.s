/*s: time/arm/time_raspi2.s */
#include "mem.h"
#include "arm.h"
#include "arminstr.ha"

/*s: function arch_lcycles(raspberry pi2)(arm) */
TEXT arch_lcycles(SB), 1, $-4
    MRC CpSC, 0, R0, C(CpCLD), C(CpCLDcyc), 0
    RET
/*e: function arch_lcycles(raspberry pi2)(arm) */

/*e: time/arm/time_raspi2.s */
