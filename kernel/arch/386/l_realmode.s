#include "mem.h"
#include "x16.h"

//*****************************************************************************
// Helpers
//*****************************************************************************

/*
 * Save registers.
 */
TEXT saveregs(SB), $0
        /* appease 8l */
        SUBL $32, SP
        POPL AX
        POPL AX
        POPL AX
        POPL AX
        POPL AX
        POPL AX
        POPL AX
        POPL AX
        
        PUSHL   AX
        PUSHL   BX
        PUSHL   CX
        PUSHL   DX
        PUSHL   BP
        PUSHL   DI
        PUSHL   SI
        PUSHFL

        XCHGL   32(SP), AX      /* swap return PC and saved flags */
        XCHGL   0(SP), AX
        XCHGL   32(SP), AX
        RET

TEXT restoreregs(SB), $0
        /* appease 8l */
        PUSHL   AX
        PUSHL   AX
        PUSHL   AX
        PUSHL   AX
        PUSHL   AX
        PUSHL   AX
        PUSHL   AX
        PUSHL   AX
        ADDL    $32, SP
        
        XCHGL   32(SP), AX      /* swap return PC and saved flags */
        XCHGL   0(SP), AX
        XCHGL   32(SP), AX

        POPFL
        POPL    SI
        POPL    DI
        POPL    BP
        POPL    DX
        POPL    CX
        POPL    BX
        POPL    AX
        RET


//*****************************************************************************
// Functions
//*****************************************************************************
                
/*
 * Assumed to be in protected mode at time of call.
 * Switch to real mode, execute an interrupt, and
 * then switch back to protected mode.  
 *
 * Assumes:
 *
 *      - no device interrupts are going to come in
 *      - 0-16MB is identity mapped in page tables
 *      - realmode() has copied us down from 0x100000 to 0x8000
 *      - can use code segment 0x0800 in real mode
 *              to get at l.s code
 *      - l.s code is less than 1 page
 */
#define RELOC   (RMCODE-KTZERO)

TEXT realmodeidtptr(SB), $0
        WORD    $(4*256-1)
        LONG    $0

TEXT realmode0(SB), $0
        CALL    saveregs(SB)

        /* switch to low code address */
        LEAL    physcode-KZERO(SB), AX
        JMP *AX

TEXT physcode(SB), $0

        /* switch to low stack */
        MOVL    SP, AX
        MOVL    $0x7C00, SP
        PUSHL   AX

        /* change gdt to physical pointer */
        MOVL    m0rgdtptr-KZERO(SB), GDTR

        /* load IDT with real-mode version*/
        MOVL    realmodeidtptr-KZERO(SB), IDTR

        /* edit INT $0x00 instruction below */
        MOVL    $(RMUADDR-KZERO+48), AX /* &rmu.trap */
        MOVL    (AX), AX
        MOVB    AX, realmodeintrinst+(-KZERO+1+RELOC)(SB)

        /* disable paging */
        MOVL    CR0, AX
        ANDL    $0x7FFFFFFF, AX
        MOVL    AX, CR0
        /* JMP .+2 to clear prefetch queue*/
        BYTE $0xEB; BYTE $0x00

        /* jump to 16-bit code segment */
/*      JMPFAR  SELECTOR(KESEG16, SELGDT, 0):$again16bit(SB) /**/
         BYTE   $0xEA
         LONG   $again16bit-KZERO(SB)
         WORD   $SELECTOR(KESEG16, SELGDT, 0)

TEXT again16bit(SB), $0
        /*
         * Now in 16-bit compatibility mode.
         * These are 32-bit instructions being interpreted
         * as 16-bit instructions.  I'm being lazy and
         * not using the macros because I know when
         * the 16- and 32-bit instructions look the same
         * or close enough.
         */

        /* disable protected mode and jump to real mode cs */
        OPSIZE; MOVL CR0, AX
        OPSIZE; XORL BX, BX
        OPSIZE; INCL BX
        OPSIZE; XORL BX, AX
        OPSIZE; MOVL AX, CR0

        /* JMPFAR 0x0800:now16real */
         BYTE $0xEA
         WORD   $now16real-KZERO(SB)
         WORD   $0x0800

TEXT now16real(SB), $0
        /* copy the registers for the bios call */
        LWI(0x0000, rAX)
        MOVW    AX,SS
        LWI(RMUADDR, rBP)
        
        /* offsets are in Ureg */
        LXW(44, xBP, rAX)
        MOVW    AX, DS
        LXW(40, xBP, rAX)
        MOVW    AX, ES

        OPSIZE; LXW(0, xBP, rDI)
        OPSIZE; LXW(4, xBP, rSI)
        OPSIZE; LXW(16, xBP, rBX)
        OPSIZE; LXW(20, xBP, rDX)
        OPSIZE; LXW(24, xBP, rCX)
        OPSIZE; LXW(28, xBP, rAX)

        CLC

TEXT realmodeintrinst(SB), $0
        INT $0x00
        CLI                     /* who knows what evil the bios got up to */

        /* save the registers after the call */

        LWI(0x7bfc, rSP)
        OPSIZE; PUSHFL
        OPSIZE; PUSHL AX

        LWI(0, rAX)
        MOVW    AX,SS
        LWI(RMUADDR, rBP)
        
        OPSIZE; SXW(rDI, 0, xBP)
        OPSIZE; SXW(rSI, 4, xBP)
        OPSIZE; SXW(rBX, 16, xBP)
        OPSIZE; SXW(rDX, 20, xBP)
        OPSIZE; SXW(rCX, 24, xBP)
        OPSIZE; POPL AX
        OPSIZE; SXW(rAX, 28, xBP)

        MOVW    DS, AX
        OPSIZE; SXW(rAX, 44, xBP)
        MOVW    ES, AX
        OPSIZE; SXW(rAX, 40, xBP)

        OPSIZE; POPL AX
        OPSIZE; SXW(rAX, 64, xBP)       /* flags */

        /* re-enter protected mode and jump to 32-bit code */
        OPSIZE; MOVL $1, AX
        OPSIZE; MOVL AX, CR0
        
/*      JMPFAR  SELECTOR(KESEG, SELGDT, 0):$again32bit(SB) /**/
         OPSIZE
         BYTE $0xEA
         LONG   $again32bit-KZERO(SB)
         WORD   $SELECTOR(KESEG, SELGDT, 0)

TEXT again32bit(SB), $0
        MOVW    $SELECTOR(KDSEG, SELGDT, 0),AX
        MOVW    AX,DS
        MOVW    AX,SS
        MOVW    AX,ES
        MOVW    AX,FS
        MOVW    AX,GS

        /* enable paging and jump to kzero-address code */
        MOVL    CR0, AX
        ORL     $0x80010000, AX /* PG|WP */
        MOVL    AX, CR0
        LEAL    again32kzero(SB), AX
        JMP*    AX

TEXT again32kzero(SB), $0
        /* breathe a sigh of relief - back in 32-bit protected mode */

        /* switch to old stack */       
        PUSHL   AX      /* match popl below for 8l */
        MOVL    $0x7BFC, SP
        POPL    SP

        /* restore idt */
        MOVL    m0idtptr(SB),IDTR

        /* restore gdt */
        MOVL    m0gdtptr(SB), GDTR

        CALL    restoreregs(SB)
        RET

/*
 * BIOS32.
 */
TEXT bios32call(SB), $0
        MOVL    ci+0(FP), BP
        MOVL    0(BP), AX
        MOVL    4(BP), BX
        MOVL    8(BP), CX
        MOVL    12(BP), DX
        MOVL    16(BP), SI
        MOVL    20(BP), DI
        PUSHL   BP

        MOVL    12(SP), BP                      /* ptr */
        BYTE $0xFF; BYTE $0x5D; BYTE $0x00      /* CALL FAR 0(BP) */

        POPL    BP
        MOVL    DI, 20(BP)
        MOVL    SI, 16(BP)
        MOVL    DX, 12(BP)
        MOVL    CX, 8(BP)
        MOVL    BX, 4(BP)
        MOVL    AX, 0(BP)

        XORL    AX, AX
        JCC     _bios32xxret
        INCL    AX

_bios32xxret:
        RET

