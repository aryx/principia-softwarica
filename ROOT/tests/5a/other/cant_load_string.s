TEXT foo(SB), $4
        // What does it mean?? address of hello string generated on the fly?
        MOVW $"hello", R1
        RET
        