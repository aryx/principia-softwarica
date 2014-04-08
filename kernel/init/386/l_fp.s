/*
 * Some machine instructions not handled by 8[al].
 */
#define FXSAVE		BYTE $0x0f; BYTE $0xae; BYTE $0x00  /* SSE FP save */
#define FXRSTOR		BYTE $0x0f; BYTE $0xae; BYTE $0x08  /* SSE FP restore */

        
/*
 * Floating point.
 * Note: the encodings for the FCLEX, FINIT, FSAVE, FSTCW, FSENV and FSTSW
 * instructions do NOT have the WAIT prefix byte (i.e. they act like their
 * FNxxx variations) so WAIT instructions must be explicitly placed in the
 * code as necessary.
 */
#define	FPOFF(l)						 ;\
	MOVL	CR0, AX 					 ;\
	ANDL	$0xC, AX			/* EM, TS */	 ;\
	CMPL	AX, $0x8					 ;\
	JEQ 	l						 ;\
	WAIT							 ;\
l:								 ;\
	MOVL	CR0, AX						 ;\
	ANDL	$~0x4, AX			/* EM=0 */	 ;\
	ORL	$0x28, AX			/* NE=1, TS=1 */ ;\
	MOVL	AX, CR0

#define	FPON							 ;\
	MOVL	CR0, AX						 ;\
	ANDL	$~0xC, AX			/* EM=0, TS=0 */ ;\
	MOVL	AX, CR0

TEXT fpon(SB), $0				/* enable */
	FPON
	RET

TEXT fpoff(SB), $0				/* disable */
	FPOFF(l1)
	RET

TEXT fpinit(SB), $0				/* enable and init */
	FPON
	FINIT
	WAIT
	/* setfcr(FPPDBL|FPRNR|FPINVAL|FPZDIV|FPOVFL) */
	/* note that low 6 bits are masks, not enables, on this chip */
	PUSHW	$0x0232
	FLDCW	0(SP)
	POPW	AX
	WAIT
	RET

TEXT fpx87save(SB), $0				/* save state and disable */
	MOVL	p+0(FP), AX
	FSAVE	0(AX)				/* no WAIT */
	FPOFF(l2)
	RET

TEXT fpx87restore(SB), $0			/* enable and restore state */
	FPON
	MOVL	p+0(FP), AX
	FRSTOR	0(AX)
	WAIT
	RET

//TEXT fpstatus(SB), $0				/* get floating point status */
//	FSTSW	AX
//	RET

TEXT fpenv(SB), $0				/* save state without waiting */
	MOVL	p+0(FP), AX
	FSTENV	0(AX)				/* also masks FP exceptions */
	RET

TEXT fpclear(SB), $0				/* clear pending exceptions */
	FPON
	FCLEX					/* no WAIT */
	FPOFF(l3)
	RET

TEXT fpssesave0(SB), $0				/* save state and disable */
	MOVL	p+0(FP), AX
	FXSAVE					/* no WAIT */
	FPOFF(l4)
	RET

TEXT fpsserestore0(SB), $0			/* enable and restore state */
	FPON
	MOVL	p+0(FP), AX
	FXRSTOR
	WAIT
	RET

        