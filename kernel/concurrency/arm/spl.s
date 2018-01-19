/*s: concurrency/arm/spl.s */
#include "mem.h"
#include "arm.h"
#include "arminstr.ha"

/*s: function [[arch_splhi]](arm) */
// bool   arch_splhi(void);
TEXT arch_splhi(SB), 1, $-4
    /*s: [[arch_splxxx()]] save caller information(arm) */
    MOVW    $(CPUADDR+4), R2        /* save caller pc in Cpu */
    MOVW    R14, 0(R2)
    /*e: [[arch_splxxx()]] save caller information(arm) */
    MOVW    CPSR, R0            /* turn off irqs (but not fiqs) */
    ORR $(PsrDirq), R0, R1
    MOVW    R1, CPSR
    RET
/*e: function [[arch_splhi]](arm) */

/*s: function [[arch_spllo]](arm) */
// bool   arch_spllo(void);
TEXT arch_spllo(SB), 1, $-4
    MOVW    CPSR, R0            /* turn on irqs and fiqs */
    BIC $(PsrDirq|PsrDfiq), R0, R1
    MOVW    R1, CPSR
    RET
/*e: function [[arch_spllo]](arm) */

/*s: function [[arch_spllx]](arm) */
// void    arch_splx(bool);
TEXT arch_splx(SB), 1, $-4
    /*s: [[arch_splxxx()]] save caller information(arm) */
    MOVW    $(CPUADDR+4), R2        /* save caller pc in Cpu */
    MOVW    R14, 0(R2)
    /*e: [[arch_splxxx()]] save caller information(arm) */
    MOVW    R0, R1              /* reset interrupt level */
    MOVW    CPSR, R0 //dead
    MOVW    R1, CPSR
    RET
/*e: function [[arch_spllx]](arm) */

TEXT spldone(SB), 1, $0             /* end marker for devkprof.c */
    RET

/*s: function [[arch_islo]](arm) */
// bool arch_islo(void);
TEXT arch_islo(SB), 1, $-4
    MOVW    CPSR, R0
    AND $(PsrDirq), R0
    EOR $(PsrDirq), R0
    RET
/*e: function [[arch_islo]](arm) */


/*s: function [[arm_arch_coherence]](arm) */
TEXT arm_arch_coherence(SB), $-4
    BARRIERS
    RET
/*e: function [[arm_arch_coherence]](arm) */


// FIQ
/*s: function [[splfhi]](arm) */
TEXT splfhi(SB), 1, $-4
    MOVW    $(CPUADDR+4), R2        /* save caller pc in Mach */
    MOVW    R14, 0(R2)

    MOVW    CPSR, R0            /* turn off irqs and fiqs */
    ORR $(PsrDirq|PsrDfiq), R0, R1
    MOVW    R1, CPSR
    RET
/*e: function [[splfhi]](arm) */

/*e: concurrency/arm/spl.s */
