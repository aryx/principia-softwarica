TEXT foo(SB), $4
        ADD R1, R2, R3
        RET


// seems accepted by 5a but offset is reset by 5l so would be better to issue
// a warning at least
TEXT bar(SB), $4
        BL foo+4(SB)
        RET
               