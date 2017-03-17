/*s: memory/arm/mmuinvalidate.s */
#include "mem.h"
#include "arm.h"
#include "arminstr.ha"

/*s: function mmuinvalidate(arm) */
/*
 * invalidate tlb
 */
TEXT mmuinvalidate(SB), 1, $-4
    MOVW    $0, R0
    MCR CpSC, 0, R0, C(CpTLB), C(CpTLBinvu), CpTLBinv
    BARRIERS
    RET
/*e: function mmuinvalidate(arm) */

/*s: function mmuinvalidateaddr(arm) */
/*
 * mmuinvalidateaddr(va)
 *   invalidate tlb entry for virtual page address va, ASID 0
 */
TEXT mmuinvalidateaddr(SB), 1, $-4
    MCR CpSC, 0, R0, C(CpTLB), C(CpTLBinvu), CpTLBinvse
    BARRIERS
    RET
/*e: function mmuinvalidateaddr(arm) */
/*e: memory/arm/mmuinvalidate.s */
