TEXT _start(SB), $0
        MOVW $setR12(SB), R12
        /* write(1, msg, len) */
	MOVW    $1, R0              /* fd = 1 (stdout) */
        MOVW    $msg(SB), R1        /* buf = &msg */
        MOVW    $13, R2          /* count = len */
        MOVW    $4, R7              /* syscall 4 = sys_write */
        SWI     $0
	/* exit(0) */
        MOVW    $0, R0              /* exit code */
        MOVW    $1, R7              /* syscall 1 = sys_exit */
        SWI     $0

GLOBL   msg(SB), $13
DATA    msg+0(SB)/8, $"Hello, w"
DATA    msg+8(SB)/5, $"orld\n"
