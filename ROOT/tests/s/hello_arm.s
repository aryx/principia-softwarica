TEXT _main(SB), $20
	// optional for this program as 5l managed to use R15 (PC) to
        // compute $hello(SB) instead of using R12
        // see also libc/arm/main9.s
	MOVW	$setR12(SB), R12
        /* prepare the system call PWRITE(1,&hello,12, 00) */
        MOVW $1, R1
        MOVW R1, 4(R13)
        MOVW $hello(SB), R1
        MOVW R1, 8(R13)
        MOVW $12, R1
        MOVW R1, 12(R13)
        MOVW $0, R1
        MOVW R1, 16(R13)
        MOVW R1, 20(R13)
        MOVW $9 /*PWRITE*/, R0
        /* system call */
        SWI $0
        BL exit(SB)
//TODO: REALLY BAD BUG
// if you comment the RET and B loop below, which mean
// BL exit(SB) should just go the next instruction, then
// weirdly some wrong code is generated and a BL 40001098 is generated
// with a big Text address resulting in a fault.
// Both kencc/5l and myplan9/5l seems to have this bug!!
// What about goken/5l???
//        RET /* not reached */
//loop:
//        B loop

TEXT exit(SB), $4
        /* prepare the system call EXITS(0) */
        MOVW $0, R1
        MOVW R1, 4(R13)
        MOVW $3 /*EXITS*/, R0
        /* system call */
        SWI $0
        RET /* not reached */

GLOBL   hello(SB), $12
DATA    hello+0(SB)/8, $"hello wo"
DATA    hello+8(SB)/4, $"rld\n"
