/*s: interrupts/arm/fault_helpers.s */
#include "mem.h"
#include "arm.h"

TEXT fsrget(SB), 1, $-4				/* data fault status */
	MRC	CpSC, 0, R0, C(CpFSR), C(0), CpFSRdata
	RET

TEXT ifsrget(SB), 1, $-4			/* instruction fault status */
	MRC	CpSC, 0, R0, C(CpFSR), C(0), CpFSRinst
	RET

TEXT farget(SB), 1, $-4				/* fault address */
	MRC	CpSC, 0, R0, C(CpFAR), C(0x0)
	RET
/*e: interrupts/arm/fault_helpers.s */
