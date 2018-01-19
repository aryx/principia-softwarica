/*s: time/arm/time_raspi1.s */
#include "mem.h"
#include "arm.h"

/*s: function [[arch_lcycles]](raspberry pi1)(arm) */
TEXT arch_lcycles(SB), 1, $-4
    MRC CpSC, 0, R0, C(CpSPM), C(CpSPMperf), CpSPMcyc
    RET
/*e: function [[arch_lcycles]](raspberry pi1)(arm) */
/*e: time/arm/time_raspi1.s */
