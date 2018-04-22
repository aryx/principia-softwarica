TEXT foo(SB), $4
        BL bar(SB)
        RET

TEXT bar(SB), $0
        MOVW $0, R0        
        RET


TEXT _main(SB), $4
        BL foo(SB)
        
                
        

        