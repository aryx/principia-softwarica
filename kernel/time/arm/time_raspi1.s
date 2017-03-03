/*s: time/arm/time_raspi1.s */
#include "mem.h"
#include "arm.h"
        
TEXT arch_lcycles(SB), 1, $-4
    MRC CpSC, 0, R0, C(CpSPM), C(CpSPMperf), CpSPMcyc
    RET

TEXT tmrget(SB), 1, $-4             /* local generic timer physical counter value */
    MOVW    $0, R1              /* not in armv6 */
    MOVW    R1, 0(R0)
    MOVW    R1, 4(R0)
    RET
/*e: time/arm/time_raspi1.s */
