#include "mem.h"
#include "arm.h"
#include "arminstr.ha"

TEXT cpidget(SB), 1, $-4			/* main ID */
	MRC	CpSC, 0, R0, C(CpID), C(0), CpIDid
	RET

TEXT fsrget(SB), 1, $-4				/* data fault status */
	MRC	CpSC, 0, R0, C(CpFSR), C(0), CpFSRdata
	RET

TEXT ifsrget(SB), 1, $-4			/* instruction fault status */
	MRC	CpSC, 0, R0, C(CpFSR), C(0), CpFSRinst
	RET

TEXT farget(SB), 1, $-4				/* fault address */
	MRC	CpSC, 0, R0, C(CpFAR), C(0x0)
	RET
