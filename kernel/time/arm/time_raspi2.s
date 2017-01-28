#include "mem.h"
#include "arm.h"
        
TEXT arch_lcycles(SB), 1, $-4
	MRC	CpSC, 0, R0, C(CpCLD), C(CpCLDcyc), 0
	RET

TEXT tmrget(SB), 1, $-4				/* local generic timer physical counter value */
	MRRC(CpSC, 0, 1, 2, CpTIMER)
	MOVM.IA [R1-R2], (R0)
	RET
