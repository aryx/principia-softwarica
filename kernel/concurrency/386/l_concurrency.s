#include "mem.h"

/*
 */
TEXT splhi(SB), $0
shi:
	PUSHFL
	POPL	AX
	TESTL	$0x200, AX
	JZ	alreadyhi
	MOVL	$(MACHADDR+0x04), CX 		/* save PC in m->splpc */
	MOVL	(SP), BX
	MOVL	BX, (CX)
alreadyhi:
	CLI
	RET

TEXT spllo(SB), $0
slo:
	PUSHFL
	POPL	AX
	TESTL	$0x200, AX
	JNZ	alreadylo
	MOVL	$(MACHADDR+0x04), CX		/* clear m->splpc */
	MOVL	$0, (CX)
alreadylo:
	STI
	RET

TEXT splx(SB), $0
	MOVL	s+0(FP), AX
	TESTL	$0x200, AX
	JNZ	slo
	JMP	shi

TEXT spldone(SB), $0
	RET

TEXT islo(SB), $0
	PUSHFL
	POPL	AX
	ANDL	$0x200, AX			/* interrupt enable flag */
	RET

/*
 * Test-And-Set
 */
TEXT tas(SB), $0
	MOVL	$0xDEADDEAD, AX
	MOVL	lock+0(FP), BX
	XCHGL	AX, (BX)			/* lock->key */
	RET

TEXT _xinc(SB), $0				/* void _xinc(long*); */
	MOVL	l+0(FP), AX
	LOCK;	INCL 0(AX)
	RET

TEXT _xdec(SB), $0				/* long _xdec(long*); */
	MOVL	l+0(FP), BX
	XORL	AX, AX
	LOCK;	DECL 0(BX)
	JLT	_xdeclt
	JGT	_xdecgt
	RET
_xdecgt:
	INCL	AX
	RET
_xdeclt:
	DECL	AX
	RET
