/*s: l_io.s */
/*
 * Some machine instructions not handled by 8[al].
 */
#define OP16            BYTE $0x66
        
/*
 * Port I/O.
 *      in[bsl]         input a byte|short|long
 *      ins[bsl]        input a string of bytes|shorts|longs
 *      out[bsl]        output a byte|short|long
 *      outs[bsl]       output a string of bytes|shorts|longs
 */
TEXT inb(SB), $0
        MOVL    port+0(FP), DX
        XORL    AX, AX
        INB
        RET

TEXT ins(SB), $0
        MOVL    port+0(FP), DX
        XORL    AX, AX
        OP16;   INL
        RET

TEXT inss(SB), $0
        MOVL    port+0(FP), DX
        MOVL    address+4(FP), DI
        MOVL    count+8(FP), CX
        CLD
        REP;    OP16; INSL
        RET

TEXT inl(SB), $0
        MOVL    port+0(FP), DX
        INL
        RET

TEXT outb(SB), $0
        MOVL    port+0(FP), DX
        MOVL    byte+4(FP), AX
        OUTB
        RET

TEXT outs(SB), $0
        MOVL    port+0(FP), DX
        MOVL    short+4(FP), AX
        OP16;   OUTL
        RET

TEXT outss(SB), $0
        MOVL    port+0(FP), DX
        MOVL    address+4(FP), SI
        MOVL    count+8(FP), CX
        CLD
        REP;    OP16; OUTSL
        RET

TEXT outl(SB), $0
        MOVL    port+0(FP), DX
        MOVL    long+4(FP), AX
        OUTL
        RET
/*e: l_io.s */
