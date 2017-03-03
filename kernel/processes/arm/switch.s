/*s: processes/arm/switch.s */
TEXT arch_setlabel(SB), 1, $-4
	MOVW	R13, 0(R0)		/* sp */
	MOVW	R14, 4(R0)		/* pc */
	MOVW	$0, R0
	RET

TEXT arch_gotolabel(SB), 1, $-4
	MOVW	0(R0), R13		/* sp */
	MOVW	4(R0), R14		/* pc */
	MOVW	$1, R0
	RET

TEXT getcallerpc(SB), 1, $-4
	MOVW	0(R13), R0
	RET
/*e: processes/arm/switch.s */
