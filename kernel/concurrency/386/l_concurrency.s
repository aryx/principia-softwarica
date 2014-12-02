/*s: l_concurrency.s */
#include "mem.h"

/*s: function splhi(x86) */
// bool   splhi(void);
TEXT splhi(SB), $0
shi:
        PUSHFL
        POPL    AX
        TESTL   $0x200, AX /* pad: interrupt bit */
        JZ      alreadyhi
        MOVL    $(CPUADDR+0x04), CX            /* save PC in cpu->splpc */
        MOVL    (SP), BX
        MOVL    BX, (CX)
alreadyhi:
        CLI
        RET
/*e: function splhi(x86) */

/*s: function spllo(x86) */
// bool   spllo(void);
TEXT spllo(SB), $0
slo:
        PUSHFL
        POPL    AX
        TESTL   $0x200, AX
        JNZ     alreadylo
        MOVL    $(CPUADDR+0x04), CX            /* clear cpu->splpc */
        MOVL    $0, (CX)
alreadylo:
        STI
        RET
/*e: function spllo(x86) */

/*s: function splx(x86) */
// void    splx(bool);
TEXT splx(SB), $0
        MOVL    s+0(FP), AX
        TESTL   $0x200, AX
        JNZ     slo
        JMP     shi
/*e: function splx(x86) */

/*s: function islo(x86) */
// bool islo(void);
TEXT islo(SB), $0
        PUSHFL
        POPL    AX
        ANDL    $0x200, AX                      /* interrupt enable flag */
        RET
/*e: function islo(x86) */

/*
 * Test-And-Set.
 */
/*s: function tas(x86) */
TEXT tas(SB), $0
        MOVL    $0xDEADDEAD, AX
        MOVL    l+0(FP), BX
        XCHGL   AX, (BX)                        /* lock->key */
        RET
/*e: function tas(x86) */

/*s: function _xinc(x86) */
/* void _xinc(long*); */
TEXT _xinc(SB), $0
        MOVL    l+0(FP), AX
        LOCK;   INCL 0(AX)
        RET
/*e: function _xinc(x86) */

/*s: function _xdec(x86) */
/* long _xdec(long*); */
TEXT _xdec(SB), $0
        MOVL    l+0(FP), BX
        XORL    AX, AX
        LOCK;   DECL 0(BX)
        JLT     _xdeclt
        JGT     _xdecgt
        RET
_xdecgt:
        INCL    AX
        RET
_xdeclt:
        DECL    AX
        RET
/*e: function _xdec(x86) */
/*e: l_concurrency.s */
