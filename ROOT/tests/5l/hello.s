TEXT _main(SB), $20
        /* prepare the system call PWRITE(1,&hello,12, 00) */
        MOVW $1, R1
        MOVW R1, 4(R13)
        MOVW $hello(SB), R1
        MOVW R1, 8(R13)
        MOVW $12, R1
        MOVW R1, 12(R13)
        MOVW $0, R1
        MOVW R1, 16(R13)
        MOVW R1, 20(R13)
        MOVW $9 /*PWRITE*/, R0
        /* system call */
        SWI $0
        BL exit(SB)

TEXT exit(SB), $4
        /* prepare the system call EXITS(0) */
        MOVW $0, R1
        MOVW R1, 4(R13)
        MOVW $3 /*EXITS*/, R0
        /* system call */
        SWI $0
        RET /* not reached */
