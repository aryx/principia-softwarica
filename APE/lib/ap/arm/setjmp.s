arg=0
link=14
sp=13

TEXT	setjmp(SB), 1, $-4
	MOVW	R(sp), (R(arg+0))
	MOVW	R(link), 4(R(arg+0))
	MOVW	$0, R0
	RET

TEXT	sigsetjmp(SB), 1, $-4
	MOVW	savemask+4(FP), R(arg+2)
	MOVW	R(arg+2), 0(R(arg+0))
	MOVW	$_psigblocked(SB), R(arg+2)
	MOVW	R2, 4(R(arg+0))
	MOVW	R(sp), 8(R(arg+0))
	MOVW	R(link), 12(R(arg+0))
	MOVW	$0, R(arg+0)
	RET
