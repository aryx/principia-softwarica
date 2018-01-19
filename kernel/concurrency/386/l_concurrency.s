/*s: l_concurrency.s */
#include "mem.h"

/*s: function [[splhi]](x86) */
// bool   arch_splhi(void);
TEXT arch_splhi(SB), $0
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
/*e: function [[splhi]](x86) */

/*s: function [[spllo]](x86) */
// bool   arch_spllo(void);
TEXT arch_spllo(SB), $0
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
/*e: function [[spllo]](x86) */

/*s: function [[splx]](x86) */
// void    arch_splx(bool);
TEXT arch_splx(SB), $0
        MOVL    s+0(FP), AX
        TESTL   $0x200, AX
        JNZ     slo
        JMP     shi
/*e: function [[splx]](x86) */

/*s: function [[islo]](x86) */
// bool arch_islo(void);
TEXT arch_islo(SB), $0
        PUSHFL
        POPL    AX
        ANDL    $0x200, AX                      /* interrupt enable flag */
        RET
/*e: function [[islo]](x86) */

/*
 * Test-And-Set.
 */
/*s: function [[tas]](x86) */
TEXT arch_tas(SB), $0
        MOVL    $0xDEADDEAD, AX
        MOVL    l+0(FP), BX
        XCHGL   AX, (BX)                        /* lock->key */
        RET
/*e: function [[tas]](x86) */

/*s: function [[_xinc]](x86) */
/* void arch_xinc(long*); */
TEXT arch_xinc(SB), $0
        MOVL    l+0(FP), AX
        LOCK;   INCL 0(AX)
        RET
/*e: function [[_xinc]](x86) */

/*s: function [[_xdec]](x86) */
/* long arch_xdec(long*); */
TEXT arch_xdec(SB), $0
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
/*e: function [[_xdec]](x86) */

// cmpswap

// cmpswap386 is in concurrenc.c
        
TEXT cmpswap486(SB), $0
        MOVL    addr+0(FP), BX
        MOVL    old+4(FP), AX
        MOVL    new+8(FP), CX
        LOCK
        BYTE $0x0F; BYTE $0xB1; BYTE $0x0B      /* CMPXCHGL CX, (BX) */
        JNZ didnt
        MOVL    $1, AX
        RET
didnt:
        XORL    AX,AX
        RET

// coherence

#define CPUID           BYTE $0x0F; BYTE $0xA2  /* CPUID, argument in AX */

/*s: function [[mb386]](x86) */
TEXT mb386(SB), $0
        POPL    AX                              /* return PC */
        PUSHFL
        PUSHL   CS
        PUSHL   AX
        IRETL
/*e: function [[mb386]](x86) */

/*s: function [[mb586]](x86) */
TEXT mb586(SB), $0
        XORL    AX, AX
        CPUID
        RET
/*e: function [[mb586]](x86) */

/*s: function [[mfence]](x86) */
TEXT mfence(SB), $0
        BYTE $0x0f
        BYTE $0xae
        BYTE $0xf0
        RET
/*e: function [[mfence]](x86) */

/*e: l_concurrency.s */
