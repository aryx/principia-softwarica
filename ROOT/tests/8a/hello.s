TEXT _main(SB), $20
        JMP later
        JMP loop /* not reached */
later:
        /* fill missing characters for hello */
        MOVL $hello(SB), BX
        MOVL $'W', AX
        MOVB AX, 6(BX)
        MOVL $'o', AX
        MOVB AX, 7(BX)
        MOVL $'r', AX
        MOVB AX, 8(BX)
        MOVL $'l', AX
        MOVB AX, 9(BX)
        MOVL $'d', AX
        MOVB AX, 10(BX)
        MOVL $'\n', AX
        MOVB AX, 11(BX)
        /* prepare the system call PWRITE(1,&hello,12, 00) */
        MOVL $1, AX
        MOVL AX, 4(SP)
        MOVL $hello(SB), AX
        MOVL AX, 8(SP)
        MOVL $12, AX
        MOVL AX, 12(SP)
        MOVL $0, AX
        MOVL AX, 16(SP)
        MOVL AX, 20(SP)
        MOVL $11 /*PWRITE*/, AX
        /* system call */
        INT $64
        CALL exit(SB)
        RET /* not reached */
loop:
        JMP loop


TEXT exit(SB), $4
        /* prepare the system call EXITS(0) */
        MOVL $0, AX
        MOVL AX, 4(SP)
        MOVL $3 /*EXITS*/, AX
        /* system call */
        INT $64
        RET /* not reached */


GLOBL   hello(SB), $12
DATA    hello+0(SB)/6, $"Hello "
