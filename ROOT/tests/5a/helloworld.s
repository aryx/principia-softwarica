TEXT _main(SB), $20
        B later
        B loop /* not reached */
later:
        /* fill missing characters for hello */
        MOVW $hello(SB), R2
        MOVW $'W', R1        
        MOVB R1, 6(R2)
        MOVW $'o', R1        
        MOVB R1, 7(R2)
        MOVW $'r', R1        
        MOVB R1, 8(R2)
        MOVW $'l', R1        
        MOVB R1, 9(R2)
        MOVW $'d', R1        
        MOVB R1, 10(R2)
        MOVW $'\n', R1
        MOVB R1, 11(R2)
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
        RET /* not reached */
loop:
        B loop
        

TEXT exit(SB), $4
        /* prepare the system call EXITS(0) */
        MOVW $0, R1
        MOVW R1, 4(R13)
        MOVW $3 /*EXITS*/, R0
        /* system call */
        SWI $0
        RET /* not reached */
        

/* fake will have hash which puts it after, faker before! */
GLOBL   fake(SB), $64
DATA    fake+0(SB)/6, $"Faker "        

GLOBL   hello(SB), $12
DATA    hello+0(SB)/6, $"Hello "

