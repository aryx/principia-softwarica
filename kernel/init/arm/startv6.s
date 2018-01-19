/*s: init/arm/startv6.s */
/*
 * Broadcom bcm2835 SoC, as used in Raspberry Pi
 * arm1176jzf-s processor (armv6)
 */
#include "mem.h"
#include "arm.h"
#include "arminstr.ha"

/*s: function [[armstart]](raspberry pi1)(arm) */
TEXT armstart(SB), 1, $-4

    /*
     * disable the mmu and L1 caches
     * invalidate caches and tlb
     */
    MRC CpSC, 0, R1, C(CpCONTROL), C(0), CpMainctl
    BIC $(CpCmmu), R1
    /*s: [[armstart()]] disable features(arm) */
    BIC $(CpCdcache|CpCicache), R1
    /*e: [[armstart()]] disable features(arm) */
    MCR CpSC, 0, R1, C(CpCONTROL), C(0), CpMainctl

    /*s: [[armstart()]] invalidate caches(arm) */
    MCR CpSC, 0, R0, C(CpCACHE), C(CpCACHEinvu), CpCACHEall
    /*e: [[armstart()]] invalidate caches(arm) */
    MCR CpSC, 0, R0, C(CpTLB), C(CpTLBinvu), CpTLBinv
    ISB

    /*
     * clear mach and page tables
     */
    MOVW    $PADDR(CPUADDR), R1
    MOVW    $PADDR(KTZERO), R2
_ramZ:
    MOVW    R0, (R1)
    ADD $4, R1
    CMP R1, R2
    BNE _ramZ

    /*
     * start stack at top of CPUADDR (physical addr)
     * set up page tables for kernel
     */
    MOVW    $PADDR(CPUADDR+CPUSIZE-4), R13 // done already in start.s
    MOVW    $PADDR(L1), R0

    BL  mmuinit(SB)

    /*
     * set up domain access control and page table base
     */
    MOVW    $Client, R1
    MCR CpSC, 0, R1, C(CpDAC), C(0)

    MOVW    $PADDR(L1), R1
    MCR CpSC, 0, R1, C(CpTTB), C(0)

    /*
     * enable caches, mmu, and high vectors
     */
    MRC CpSC, 0, R0, C(CpCONTROL), C(0), CpMainctl
    ORR $(CpCmmu|CpChv), R0
    /*s: [[armstart()]] reenable features(arm) */
    ORR $(CpCdcache|CpCicache), R0
    /*e: [[armstart()]] reenable features(arm) */
    MCR CpSC, 0, R0, C(CpCONTROL), C(0), CpMainctl
    ISB

    /*
     * switch SB, SP, and PC into KZERO space
     */
    MOVW    $setR12(SB), R12
    MOVW    $(CPUADDR+CPUSIZE-4), R13

    MOVW    $_startpg(SB), R15
/*e: function [[armstart]](raspberry pi1)(arm) */

/*s: function [[_startpg]](raspberry pi1)(arm) */
TEXT _startpg(SB), 1, $-4

    /*s: [[_startpg()]] enable cycle counter(raspberry pi1)(arm) */
    /*
     * enable cycle counter
     */
    MOVW    $1, R1
    MCR CpSC, 0, R1, C(CpSPM), C(CpSPMperf), CpSPMctl
    /*e: [[_startpg()]] enable cycle counter(raspberry pi1)(arm) */
    /*
     * call main and loop forever if it returns
     */
    BL  main(SB)

    // Unreached            
    B   0(PC)
    BL  _div(SB)        /* hack to load _div, etc. */
/*e: function [[_startpg]](raspberry pi1)(arm) */


        
/*s: function [[arch_idlehands]](raspberry pi1)(arm) */
TEXT arch_idlehands(SB), $-4
    MOVW    CPSR, R3
    ORR $(PsrDirq|PsrDfiq), R3, R1      /* splfhi */
    MOVW    R1, CPSR

    DSB
    MOVW    nrdy(SB), R0
    CMP $0, R0
    MCR.EQ  CpSC, 0, R0, C(CpCACHE), C(CpCACHEintr), CpCACHEwait
    DSB

    MOVW    R3, CPSR            /* splx */
    RET
/*e: function [[arch_idlehands]](raspberry pi1)(arm) */
/*e: init/arm/startv6.s */
