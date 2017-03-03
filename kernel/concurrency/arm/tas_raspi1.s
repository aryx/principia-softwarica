/*s: concurrency/arm/tas_raspi1.s */
TEXT	arch_tas(SB), $-4
TEXT	_tas(SB), $-4
	MOVW	R0,R1
	MOVW	$1,R0
	SWPW	R0,(R1)			/* fix: deprecated in armv6 */
	RET
/*e: concurrency/arm/tas_raspi1.s */
