/*s: l_concurrency.s */
#include "mem.h"

/*s: function splhi */
// int   splhi(void);
TEXT splhi(SB), $0
shi:
        PUSHFL
        POPL    AX
        TESTL   $0x200, AX
        JZ      alreadyhi
        MOVL    $(CPUADDR+0x04), CX            /* save PC in cpu->splpc */
        MOVL    (SP), BX
        MOVL    BX, (CX)
alreadyhi:
        CLI
        RET
/*e: function splhi */

/*s: function spllo */
// int   spllo(void);
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
/*e: function spllo */

/*s: function splx */
// void    splx(int);
TEXT splx(SB), $0
        MOVL    s+0(FP), AX
        TESTL   $0x200, AX
        JNZ     slo
        JMP     shi
/*e: function splx */

/*s: function islo */
// bool islo(void);
TEXT islo(SB), $0
        PUSHFL
        POPL    AX
        ANDL    $0x200, AX                      /* interrupt enable flag */
        RET
/*e: function islo */

/*
 * Test-And-Set.
 */
/*s: function tas */
TEXT tas(SB), $0
        MOVL    $0xDEADDEAD, AX
        MOVL    lock+0(FP), BX
        // Exchange AX to lock->key. So:
        //  - if the lock was not held, lock->key was 0 and so AX will be 0
        //     and lock->key will be 0xdeaddead
        //  - if the lock was held, AX will be 0xdeaddead and lock->key will
        //    still be 0xdeaddead.
        XCHGL   AX, (BX)                        /* lock->key */
        RET
/*e: function tas */

/*s: function _xinc */
/* void _xinc(long*); */
TEXT _xinc(SB), $0
        MOVL    l+0(FP), AX
        LOCK;   INCL 0(AX)
        RET
/*e: function _xinc */

/*s: function _xdec */
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
/*e: function _xdec */
/*e: l_concurrency.s */
