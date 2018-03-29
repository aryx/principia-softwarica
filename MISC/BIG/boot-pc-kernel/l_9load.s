was in pc/l.s but not needed:
        
        /*
 * For backwards compatiblity with 9load - should go away when 9load is changed
 * 9load currently sets up the mmu, however the first 16MB of memory is identity
 * mapped, so behave as if the mmu was not setup
 */
TEXT _startKADDR(SB), $0
	MOVL	$_startPADDR(SB), AX
	ANDL	$~KZERO, AX
	JMP*	AX
