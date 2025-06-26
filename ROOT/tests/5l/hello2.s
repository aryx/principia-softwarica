TEXT main(SB), $8
        /* fprint(1,&hello) */
        MOVW $1, R0
        MOVW $hello(SB), R1
        MOVW R1, 8(R13)
        BL fprint(SB)
        /* exit(0) */
        MOVW $0, R0
        BL exits(SB)
