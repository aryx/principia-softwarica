
#include "/sys/src/libc/9syscall/sys.h"        
        
TEXT _main(SB), 0, $20
        MOVW $NOP, R0
        MOVW $3, R1
        SWI

        MOVW $1, R1
        MOVW R1, 4(R13)
        MOVW $hello(SB), R1
        MOVW R1, 8(R13)
        MOVW $12, R1
        MOVW R1, 12(R13)
        MOVW $0, R1
        MOVW R1, 16(R13)
        MOVW $PWRITE, R0
        SWI

        MOVW $end(SB), R1
        MOVW R1, 4(R13)                
        MOVW $EXITS, R0
        SWI

GLOBL   hello(SB), $16
DATA    hello+0(SB)/8, $"hello wo"
DATA    hello+8(SB)/8, $"rld\n\z\z\z\z"

GLOBL   end(SB), $8
DATA    end+0(SB)/8, $"end\z\z\z\z\z"
                        

/* http://sysdigcloud.com/fascinating-world-linux-system-calls/ */
