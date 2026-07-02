TEXT _main(SB), $20
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

TEXT exit(SB), $4
        /* prepare the system call EXITS(0) */
        MOVL $0, AX
        MOVL AX, 4(SP)
        MOVL $3 /*EXITS*/, AX
        /* system call */
        INT $64
        RET /* not reached */
