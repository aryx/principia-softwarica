/*s: l_trap.s */
#include "mem.h"

/*s: function [[_strayintr]](x86) */
/*
 * Interrupt/exception handling.
 * Each entry in the vector table calls either _strayintr or _strayintrx depending
 * on whether an error code has been automatically pushed onto the stack
 * (_strayintrx) or not, in which case a dummy entry must be pushed before retrieving
 * the trap type from the vector table entry and placing it on the stack as part
 * of the Ureg structure.
 * The size of each entry in the vector table (6 bytes) is known in trapinit().
 */
TEXT _strayintr(SB), $0
        PUSHL   AX                      /* save AX */
        MOVL    4(SP), AX               /* return PC from vectortable(SB) */
        JMP     intrcommon
/*e: function [[_strayintr]](x86) */

/*s: function [[_strayintrx]](x86) */
TEXT _strayintrx(SB), $0
        XCHGL   AX, (SP)                /* swap AX with vectortable CALL PC */
intrcommon:
        PUSHL   DS                      /* save DS */
        PUSHL   $(KDSEL)
        POPL    DS                      /* fix up DS */
        MOVBLZX (AX), AX                /* trap type -> AX */
        XCHGL   AX, 4(SP)               /* exchange trap type with saved AX */

        PUSHL   ES                      /* save ES */
        PUSHL   $(KDSEL)
        POPL    ES                      /* fix up ES */

        PUSHL   FS                      /* save the rest of the Ureg struct */
        PUSHL   GS
        PUSHAL

        PUSHL   SP                      /* Ureg* argument to trap */
        CALL    trap(SB)
/*e: function [[_strayintrx]](x86) */

/*s: function [[forkret]](x86) */
TEXT arch__forkret(SB), $0
        POPL    AX
        POPAL
        POPL    GS
        POPL    FS
        POPL    ES
        POPL    DS
        ADDL    $8, SP                  /* pop error code and trap type */
        IRETL
/*e: function [[forkret]](x86) */

/*s: global [[vectortable]](x86) */
TEXT vectortable(SB), $0
        CALL _strayintr(SB); BYTE $0x00         /* divide error */
        CALL _strayintr(SB); BYTE $0x01         /* debug exception */
        CALL _strayintr(SB); BYTE $0x02         /* NMI interrupt */
        CALL _strayintr(SB); BYTE $0x03         /* breakpoint */
        CALL _strayintr(SB); BYTE $0x04         /* overflow */
        CALL _strayintr(SB); BYTE $0x05         /* bound */
        CALL _strayintr(SB); BYTE $0x06         /* invalid opcode */
        CALL _strayintr(SB); BYTE $0x07         /* no coprocessor available */
        CALL _strayintrx(SB); BYTE $0x08        /* double fault */
        CALL _strayintr(SB); BYTE $0x09         /* coprocessor segment overflow */
        CALL _strayintrx(SB); BYTE $0x0A        /* invalid TSS */
        CALL _strayintrx(SB); BYTE $0x0B        /* segment not available */
        CALL _strayintrx(SB); BYTE $0x0C        /* stack exception */
        CALL _strayintrx(SB); BYTE $0x0D        /* general protection error */
        CALL _strayintrx(SB); BYTE $0x0E        /* page fault */
        CALL _strayintr(SB); BYTE $0x0F         /*  */
        CALL _strayintr(SB); BYTE $0x10         /* coprocessor error */
        CALL _strayintrx(SB); BYTE $0x11        /* alignment check */
        CALL _strayintr(SB); BYTE $0x12         /* machine check */
        CALL _strayintr(SB); BYTE $0x13
        CALL _strayintr(SB); BYTE $0x14
        CALL _strayintr(SB); BYTE $0x15
        CALL _strayintr(SB); BYTE $0x16
        CALL _strayintr(SB); BYTE $0x17
        CALL _strayintr(SB); BYTE $0x18
        CALL _strayintr(SB); BYTE $0x19
        CALL _strayintr(SB); BYTE $0x1A
        CALL _strayintr(SB); BYTE $0x1B
        CALL _strayintr(SB); BYTE $0x1C
        CALL _strayintr(SB); BYTE $0x1D
        CALL _strayintr(SB); BYTE $0x1E
        CALL _strayintr(SB); BYTE $0x1F
        CALL _strayintr(SB); BYTE $0x20         /* VectorLAPIC */
        CALL _strayintr(SB); BYTE $0x21
        CALL _strayintr(SB); BYTE $0x22
        CALL _strayintr(SB); BYTE $0x23
        CALL _strayintr(SB); BYTE $0x24
        CALL _strayintr(SB); BYTE $0x25
        CALL _strayintr(SB); BYTE $0x26
        CALL _strayintr(SB); BYTE $0x27
        CALL _strayintr(SB); BYTE $0x28
        CALL _strayintr(SB); BYTE $0x29
        CALL _strayintr(SB); BYTE $0x2A
        CALL _strayintr(SB); BYTE $0x2B
        CALL _strayintr(SB); BYTE $0x2C
        CALL _strayintr(SB); BYTE $0x2D
        CALL _strayintr(SB); BYTE $0x2E
        CALL _strayintr(SB); BYTE $0x2F
        CALL _strayintr(SB); BYTE $0x30
        CALL _strayintr(SB); BYTE $0x31
        CALL _strayintr(SB); BYTE $0x32
        CALL _strayintr(SB); BYTE $0x33
        CALL _strayintr(SB); BYTE $0x34
        CALL _strayintr(SB); BYTE $0x35
        CALL _strayintr(SB); BYTE $0x36
        CALL _strayintr(SB); BYTE $0x37
        CALL _strayintr(SB); BYTE $0x38
        CALL _strayintr(SB); BYTE $0x39
        CALL _strayintr(SB); BYTE $0x3A
        CALL _strayintr(SB); BYTE $0x3B
        CALL _strayintr(SB); BYTE $0x3C
        CALL _strayintr(SB); BYTE $0x3D
        CALL _strayintr(SB); BYTE $0x3E
        CALL _strayintr(SB); BYTE $0x3F
        CALL _syscallintr(SB); BYTE $0x40       /* VectorSYSCALL */
        CALL _strayintr(SB); BYTE $0x41
        CALL _strayintr(SB); BYTE $0x42
        CALL _strayintr(SB); BYTE $0x43
        CALL _strayintr(SB); BYTE $0x44
        CALL _strayintr(SB); BYTE $0x45
        CALL _strayintr(SB); BYTE $0x46
        CALL _strayintr(SB); BYTE $0x47
        CALL _strayintr(SB); BYTE $0x48
        CALL _strayintr(SB); BYTE $0x49
        CALL _strayintr(SB); BYTE $0x4A
        CALL _strayintr(SB); BYTE $0x4B
        CALL _strayintr(SB); BYTE $0x4C
        CALL _strayintr(SB); BYTE $0x4D
        CALL _strayintr(SB); BYTE $0x4E
        CALL _strayintr(SB); BYTE $0x4F
        CALL _strayintr(SB); BYTE $0x50
        CALL _strayintr(SB); BYTE $0x51
        CALL _strayintr(SB); BYTE $0x52
        CALL _strayintr(SB); BYTE $0x53
        CALL _strayintr(SB); BYTE $0x54
        CALL _strayintr(SB); BYTE $0x55
        CALL _strayintr(SB); BYTE $0x56
        CALL _strayintr(SB); BYTE $0x57
        CALL _strayintr(SB); BYTE $0x58
        CALL _strayintr(SB); BYTE $0x59
        CALL _strayintr(SB); BYTE $0x5A
        CALL _strayintr(SB); BYTE $0x5B
        CALL _strayintr(SB); BYTE $0x5C
        CALL _strayintr(SB); BYTE $0x5D
        CALL _strayintr(SB); BYTE $0x5E
        CALL _strayintr(SB); BYTE $0x5F
        CALL _strayintr(SB); BYTE $0x60
        CALL _strayintr(SB); BYTE $0x61
        CALL _strayintr(SB); BYTE $0x62
        CALL _strayintr(SB); BYTE $0x63
        CALL _strayintr(SB); BYTE $0x64
        CALL _strayintr(SB); BYTE $0x65
        CALL _strayintr(SB); BYTE $0x66
        CALL _strayintr(SB); BYTE $0x67
        CALL _strayintr(SB); BYTE $0x68
        CALL _strayintr(SB); BYTE $0x69
        CALL _strayintr(SB); BYTE $0x6A
        CALL _strayintr(SB); BYTE $0x6B
        CALL _strayintr(SB); BYTE $0x6C
        CALL _strayintr(SB); BYTE $0x6D
        CALL _strayintr(SB); BYTE $0x6E
        CALL _strayintr(SB); BYTE $0x6F
        CALL _strayintr(SB); BYTE $0x70
        CALL _strayintr(SB); BYTE $0x71
        CALL _strayintr(SB); BYTE $0x72
        CALL _strayintr(SB); BYTE $0x73
        CALL _strayintr(SB); BYTE $0x74
        CALL _strayintr(SB); BYTE $0x75
        CALL _strayintr(SB); BYTE $0x76
        CALL _strayintr(SB); BYTE $0x77
        CALL _strayintr(SB); BYTE $0x78
        CALL _strayintr(SB); BYTE $0x79
        CALL _strayintr(SB); BYTE $0x7A
        CALL _strayintr(SB); BYTE $0x7B
        CALL _strayintr(SB); BYTE $0x7C
        CALL _strayintr(SB); BYTE $0x7D
        CALL _strayintr(SB); BYTE $0x7E
        CALL _strayintr(SB); BYTE $0x7F
        CALL _strayintr(SB); BYTE $0x80         /* Vector[A]PIC */
        CALL _strayintr(SB); BYTE $0x81
        CALL _strayintr(SB); BYTE $0x82
        CALL _strayintr(SB); BYTE $0x83
        CALL _strayintr(SB); BYTE $0x84
        CALL _strayintr(SB); BYTE $0x85
        CALL _strayintr(SB); BYTE $0x86
        CALL _strayintr(SB); BYTE $0x87
        CALL _strayintr(SB); BYTE $0x88
        CALL _strayintr(SB); BYTE $0x89
        CALL _strayintr(SB); BYTE $0x8A
        CALL _strayintr(SB); BYTE $0x8B
        CALL _strayintr(SB); BYTE $0x8C
        CALL _strayintr(SB); BYTE $0x8D
        CALL _strayintr(SB); BYTE $0x8E
        CALL _strayintr(SB); BYTE $0x8F
        CALL _strayintr(SB); BYTE $0x90
        CALL _strayintr(SB); BYTE $0x91
        CALL _strayintr(SB); BYTE $0x92
        CALL _strayintr(SB); BYTE $0x93
        CALL _strayintr(SB); BYTE $0x94
        CALL _strayintr(SB); BYTE $0x95
        CALL _strayintr(SB); BYTE $0x96
        CALL _strayintr(SB); BYTE $0x97
        CALL _strayintr(SB); BYTE $0x98
        CALL _strayintr(SB); BYTE $0x99
        CALL _strayintr(SB); BYTE $0x9A
        CALL _strayintr(SB); BYTE $0x9B
        CALL _strayintr(SB); BYTE $0x9C
        CALL _strayintr(SB); BYTE $0x9D
        CALL _strayintr(SB); BYTE $0x9E
        CALL _strayintr(SB); BYTE $0x9F
        CALL _strayintr(SB); BYTE $0xA0
        CALL _strayintr(SB); BYTE $0xA1
        CALL _strayintr(SB); BYTE $0xA2
        CALL _strayintr(SB); BYTE $0xA3
        CALL _strayintr(SB); BYTE $0xA4
        CALL _strayintr(SB); BYTE $0xA5
        CALL _strayintr(SB); BYTE $0xA6
        CALL _strayintr(SB); BYTE $0xA7
        CALL _strayintr(SB); BYTE $0xA8
        CALL _strayintr(SB); BYTE $0xA9
        CALL _strayintr(SB); BYTE $0xAA
        CALL _strayintr(SB); BYTE $0xAB
        CALL _strayintr(SB); BYTE $0xAC
        CALL _strayintr(SB); BYTE $0xAD
        CALL _strayintr(SB); BYTE $0xAE
        CALL _strayintr(SB); BYTE $0xAF
        CALL _strayintr(SB); BYTE $0xB0
        CALL _strayintr(SB); BYTE $0xB1
        CALL _strayintr(SB); BYTE $0xB2
        CALL _strayintr(SB); BYTE $0xB3
        CALL _strayintr(SB); BYTE $0xB4
        CALL _strayintr(SB); BYTE $0xB5
        CALL _strayintr(SB); BYTE $0xB6
        CALL _strayintr(SB); BYTE $0xB7
        CALL _strayintr(SB); BYTE $0xB8
        CALL _strayintr(SB); BYTE $0xB9
        CALL _strayintr(SB); BYTE $0xBA
        CALL _strayintr(SB); BYTE $0xBB
        CALL _strayintr(SB); BYTE $0xBC
        CALL _strayintr(SB); BYTE $0xBD
        CALL _strayintr(SB); BYTE $0xBE
        CALL _strayintr(SB); BYTE $0xBF
        CALL _strayintr(SB); BYTE $0xC0
        CALL _strayintr(SB); BYTE $0xC1
        CALL _strayintr(SB); BYTE $0xC2
        CALL _strayintr(SB); BYTE $0xC3
        CALL _strayintr(SB); BYTE $0xC4
        CALL _strayintr(SB); BYTE $0xC5
        CALL _strayintr(SB); BYTE $0xC6
        CALL _strayintr(SB); BYTE $0xC7
        CALL _strayintr(SB); BYTE $0xC8
        CALL _strayintr(SB); BYTE $0xC9
        CALL _strayintr(SB); BYTE $0xCA
        CALL _strayintr(SB); BYTE $0xCB
        CALL _strayintr(SB); BYTE $0xCC
        CALL _strayintr(SB); BYTE $0xCD
        CALL _strayintr(SB); BYTE $0xCE
        CALL _strayintr(SB); BYTE $0xCF
        CALL _strayintr(SB); BYTE $0xD0
        CALL _strayintr(SB); BYTE $0xD1
        CALL _strayintr(SB); BYTE $0xD2
        CALL _strayintr(SB); BYTE $0xD3
        CALL _strayintr(SB); BYTE $0xD4
        CALL _strayintr(SB); BYTE $0xD5
        CALL _strayintr(SB); BYTE $0xD6
        CALL _strayintr(SB); BYTE $0xD7
        CALL _strayintr(SB); BYTE $0xD8
        CALL _strayintr(SB); BYTE $0xD9
        CALL _strayintr(SB); BYTE $0xDA
        CALL _strayintr(SB); BYTE $0xDB
        CALL _strayintr(SB); BYTE $0xDC
        CALL _strayintr(SB); BYTE $0xDD
        CALL _strayintr(SB); BYTE $0xDE
        CALL _strayintr(SB); BYTE $0xDF
        CALL _strayintr(SB); BYTE $0xE0
        CALL _strayintr(SB); BYTE $0xE1
        CALL _strayintr(SB); BYTE $0xE2
        CALL _strayintr(SB); BYTE $0xE3
        CALL _strayintr(SB); BYTE $0xE4
        CALL _strayintr(SB); BYTE $0xE5
        CALL _strayintr(SB); BYTE $0xE6
        CALL _strayintr(SB); BYTE $0xE7
        CALL _strayintr(SB); BYTE $0xE8
        CALL _strayintr(SB); BYTE $0xE9
        CALL _strayintr(SB); BYTE $0xEA
        CALL _strayintr(SB); BYTE $0xEB
        CALL _strayintr(SB); BYTE $0xEC
        CALL _strayintr(SB); BYTE $0xED
        CALL _strayintr(SB); BYTE $0xEE
        CALL _strayintr(SB); BYTE $0xEF
        CALL _strayintr(SB); BYTE $0xF0
        CALL _strayintr(SB); BYTE $0xF1
        CALL _strayintr(SB); BYTE $0xF2
        CALL _strayintr(SB); BYTE $0xF3
        CALL _strayintr(SB); BYTE $0xF4
        CALL _strayintr(SB); BYTE $0xF5
        CALL _strayintr(SB); BYTE $0xF6
        CALL _strayintr(SB); BYTE $0xF7
        CALL _strayintr(SB); BYTE $0xF8
        CALL _strayintr(SB); BYTE $0xF9
        CALL _strayintr(SB); BYTE $0xFA
        CALL _strayintr(SB); BYTE $0xFB
        CALL _strayintr(SB); BYTE $0xFC
        CALL _strayintr(SB); BYTE $0xFD
        CALL _strayintr(SB); BYTE $0xFE
        CALL _strayintr(SB); BYTE $0xFF
/*e: global [[vectortable]](x86) */
/*e: l_trap.s */
