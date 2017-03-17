/*s: concurrency/arm/tas_raspi2.s */
#include "mem.h"
#include "arm.h"
#include "arminstr.ha"

/*s: instruction DMB(arm) */
#define DMB WORD    $0xf57ff05f /* data mem. barrier; last f = SY */
/*e: instruction DMB(arm) */
        
/* tas/cas strex debugging limits; started at 10000 */
#define MAXSC 100000

TEXT    _tas(SB), $-4           /* _tas(ulong *) */
/*s: function arch_tas(raspberry pi2)(arm) */
TEXT    arch_tas(SB), $-4
    /* returns old (R0) after modifying (R0) */
    MOVW    R0,R5
    DMB

    MOVW    $1,R2       /* new value of (R0) */
    MOVW    $MAXSC, R8
tas1:
    LDREX(5,7)      /* LDREX 0(R5),R7 */
    CMP.S   $0, R7      /* old value non-zero (lock taken)? */
    BNE lockbusy    /* we lose */
    SUB.S   $1, R8
    BEQ lockloop2
    STREX(2,5,4)        /* STREX R2,(R5),R4 */
    CMP.S   $0, R4
    BNE tas1        /* strex failed? try again */
    DMB
    B   tas0
lockloop2:
    BL  abort(SB)
lockbusy:
    CLREX
tas0:
    MOVW    R7, R0      /* return old value */
    RET
/*e: function arch_tas(raspberry pi2)(arm) */
/*e: concurrency/arm/tas_raspi2.s */
