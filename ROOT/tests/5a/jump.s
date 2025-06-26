TEXT foo(SB), 0, $0
        B lbl
        RET


TEXT bar(SB), 0, $0

lbl:
     BL foobar(SB)
        
        
TEXT foobar(SB), 0, $0
        RET
        
                        
