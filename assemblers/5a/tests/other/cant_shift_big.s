TEXT foo(SB), $4
        // this is detected at linking time though right now
        SLL $33, R1, R2
//        ADD R1<<33, R2, R3
        