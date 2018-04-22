TEXT _main(SB), $0
        // should be redirected to call exit
        BL bar(SB)
        BL bar(SB)
        BL bar(SB)
        BL bar(SB)
        RET



//TEXT exit(SB), $0
//        RET
        