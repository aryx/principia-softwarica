TEXT _main(SB), $20
        MOVW etext(SB), R1
        MOVW $edata(SB), R2
        MOVW _main(SB), R3
        MOVW $_main(SB), R4
        RET
        