
#include "/sys/src/libc/9syscall/sys.h"        
        
TEXT _main(SB), $-4

        MOVW $NOP, R0
        MOVW $NOP, R0
        MOVW $NOP, R0
        MOVW $NOP, R0
        MOVW $3, R1
        SWI
loop:
        B loop
                
        