TEXT foo(SB), $4
        ADD R0, R1, R2
        RET

TEXT bar(SB), $4
        // MOVW $hello(SB), R1 makes sense, but the code below
        // generates wrong code I think as it uses the value of foo
        // which is its real_pc but still adds it like it was an offset
        // to R12
        MOVW $foo(SB), R1
        RET

TEXT _main(SB), $0
        BL foo(SB)        

GLOBL hello(SB), $4
        