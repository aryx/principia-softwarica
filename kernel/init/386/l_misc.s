/*s: l_misc.s */

/*
 * Some machine instructions not handled by 8[al].
 */
#define HLT             BYTE $0xF4
        
/*
 * Park a processor. Should never fall through a return from main to here,
 * should only be called by application processors when shutting down.
 */
TEXT idle(SB), $0
_idle:
        STI
        HLT
        JMP     _idle


        
/*
 * Attempt at power saving. -rsc
 */
TEXT halt(SB), $0
        CLI
        CMPL    nrdy(SB), $0
        JEQ     _nothingready
        STI
        RET

_nothingready:
        STI
        HLT
        RET



/*
 * Basic timing loop to determine CPU frequency.
 */
TEXT aamloop(SB), $0
        MOVL    count+0(FP), CX
_aamloop:
        AAM
        LOOP    _aamloop
        RET


//*****************************************************************************
// Misc
//*****************************************************************************

#define INVLPG  BYTE $0x0F; BYTE $0x01; BYTE $0x39      /* INVLPG (%ecx) */
TEXT invlpg(SB), $0
        /* 486+ only */
        MOVL    va+0(FP), CX
        INVLPG
        RET

#define WBINVD  BYTE $0x0F; BYTE $0x09
TEXT wbinvd(SB), $0
        WBINVD
        RET

#define RDTSC           BYTE $0x0F; BYTE $0x31  /* RDTSC, result in AX/DX (lo/hi) */
TEXT _cycles(SB), $0                            /* time stamp counter */
        RDTSC
        MOVL    vlong+0(FP), CX                 /* &vlong */
        MOVL    AX, 0(CX)                       /* lo */
        MOVL    DX, 4(CX)                       /* hi */
        RET

/*
 * stub for:
 * time stamp counter; low-order 32 bits of 64-bit cycle counter
 * Runs at fasthz/4 cycles per second (m->clkin>>3)
 */
TEXT lcycles(SB),1,$0
        RDTSC
        RET

#define RDMSR           BYTE $0x0F; BYTE $0x32  /* RDMSR, result in AX/DX (lo/hi) */
TEXT rdmsr(SB), $0                              /* model-specific register */
        MOVL    index+0(FP), CX
        RDMSR
        MOVL    vlong+4(FP), CX                 /* &vlong */
        MOVL    AX, 0(CX)                       /* lo */
        MOVL    DX, 4(CX)                       /* hi */
        RET

#define WRMSR           BYTE $0x0F; BYTE $0x30  /* WRMSR, argument in AX/DX (lo/hi) */
                
TEXT wrmsr(SB), $0
        MOVL    index+0(FP), CX
        MOVL    lo+4(FP), AX
        MOVL    hi+8(FP), DX
        WRMSR
        RET


#define CPUID           BYTE $0x0F; BYTE $0xA2  /* CPUID, argument in AX */
        
/*
 * Try to determine the CPU type which requires fiddling with EFLAGS.
 * If the Id bit can be toggled then the CPUID instruction can be used
 * to determine CPU identity and features. First have to check if it's
 * a 386 (Ac bit can't be set). If it's not a 386 and the Id bit can't be
 * toggled then it's an older 486 of some kind.
 *
 *      cpuid(fun, regs[4]);
 */
TEXT cpuid(SB), $0
        MOVL    $0x240000, AX
        PUSHL   AX
        POPFL                                   /* set Id|Ac */
        PUSHFL
        POPL    BX                              /* retrieve value */
        MOVL    $0, AX
        PUSHL   AX
        POPFL                                   /* clear Id|Ac, EFLAGS initialised */
        PUSHFL
        POPL    AX                              /* retrieve value */
        XORL    BX, AX
        TESTL   $0x040000, AX                   /* Ac */
        JZ      _cpu386                         /* can't set this bit on 386 */
        TESTL   $0x200000, AX                   /* Id */
        JZ      _cpu486                         /* can't toggle this bit on some 486 */
        /* load registers */
        MOVL    regs+4(FP), BP
        MOVL    fn+0(FP), AX                    /* cpuid function */
        MOVL    4(BP), BX
        MOVL    8(BP), CX                       /* typically an index */
        MOVL    12(BP), DX
        CPUID
        JMP     _cpuid
_cpu486:
        MOVL    $0x400, AX
        JMP     _maybezapax
_cpu386:
        MOVL    $0x300, AX
_maybezapax:
        CMPL    fn+0(FP), $1
        JE      _zaprest
        XORL    AX, AX
_zaprest:
        XORL    BX, BX
        XORL    CX, CX
        XORL    DX, DX
_cpuid:
        MOVL    regs+4(FP), BP
        MOVL    AX, 0(BP)
        MOVL    BX, 4(BP)
        MOVL    CX, 8(BP)
        MOVL    DX, 12(BP)
        RET




TEXT mb386(SB), $0
        POPL    AX                              /* return PC */
        PUSHFL
        PUSHL   CS
        PUSHL   AX
        IRETL

TEXT mb586(SB), $0
        XORL    AX, AX
        CPUID
        RET

TEXT sfence(SB), $0
        BYTE $0x0f
        BYTE $0xae
        BYTE $0xf8
        RET

TEXT lfence(SB), $0
        BYTE $0x0f
        BYTE $0xae
        BYTE $0xe8
        RET

TEXT mfence(SB), $0
        BYTE $0x0f
        BYTE $0xae
        BYTE $0xf0
        RET

//TEXT xchgw(SB), $0
//      MOVL    v+4(FP), AX
//      MOVL    p+0(FP), BX
//      XCHGW   AX, (BX)
//      RET

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

TEXT mul64fract(SB), $0
/*
 * Multiply two 64-bit number s and keep the middle 64 bits from the 128-bit result
 * See ../port/tod.c for motivation.
 */
        MOVL    r+0(FP), CX
        XORL    BX, BX                          /* BX = 0 */

        MOVL    a+8(FP), AX
        MULL    b+16(FP)                        /* a1*b1 */
        MOVL    AX, 4(CX)                       /* r2 = lo(a1*b1) */

        MOVL    a+8(FP), AX
        MULL    b+12(FP)                        /* a1*b0 */
        MOVL    AX, 0(CX)                       /* r1 = lo(a1*b0) */
        ADDL    DX, 4(CX)                       /* r2 += hi(a1*b0) */

        MOVL    a+4(FP), AX
        MULL    b+16(FP)                        /* a0*b1 */
        ADDL    AX, 0(CX)                       /* r1 += lo(a0*b1) */
        ADCL    DX, 4(CX)                       /* r2 += hi(a0*b1) + carry */

        MOVL    a+4(FP), AX
        MULL    b+12(FP)                        /* a0*b0 */
        ADDL    DX, 0(CX)                       /* r1 += hi(a0*b0) */
        ADCL    BX, 4(CX)                       /* r2 += carry */
        RET
/*e: l_misc.s */
