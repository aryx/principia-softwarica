TEXT foo(SB), $4
        MOVW $bar(SB), R1
        RET

TEXT bar(SB), $4
        ADD R1, R2, R3
        RET

        
                