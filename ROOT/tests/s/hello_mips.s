TEXT _main(SB), $20
	// very important instruction! vl assumes R30 is set correctly so that
        // $hello(SB) below is translated in using R30 and some offset.
        // see also libc/mips/main9.s
	MOVW	$setR30(SB), R30
        /* prepare the system call PWRITE(1,&hello,12, 00) */
	MOVW	$1,R1
	MOVW    R1, 4(R29)
	MOVW	$hello(SB),R2
	MOVW	R2,8(R29)
	MOVW	$12,R4
	MOVW	R4,12(R29)
	MOVW	$16(R29),R6
	MOVW	$0,0(R6)
	MOVW	$0,4(R6)

        /* system call */
	MOVW $9 /*PWRITE*/, R1
	SYSCALL
        JAL exit(SB)

TEXT exit(SB), $4
        /* prepare the system call EXITS(0) */
        MOVW $0, R1
        MOVW R1, 4(R29)
        MOVW $3 /*EXITS*/, R1
        /* system call */
        SYSCALL
        RET /* not reached */

GLOBL   hello(SB), $12
DATA    hello+0(SB)/8, $"hello wo"
DATA    hello+8(SB)/4, $"rld\n"
