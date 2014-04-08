
/*
 * Some machine instructions not handled by 8[al].
 */
#define HLT		BYTE $0xF4
        
/*
 * Park a processor. Should never fall through a return from main to here,
 * should only be called by application processors when shutting down.
 */
TEXT idle(SB), $0
_idle:
	STI
	HLT
	JMP	_idle


        
/*
 * Attempt at power saving. -rsc
 */
TEXT halt(SB), $0
	CLI
	CMPL	nrdy(SB), $0
	JEQ	_nothingready
	STI
	RET

_nothingready:
	STI
	HLT
	RET





/*
 * Basic timing loop to determine CPU frequency.
 */
TEXT aamloop(SB), $0
	MOVL	count+0(FP), CX
_aamloop:
	AAM
	LOOP	_aamloop
	RET
        