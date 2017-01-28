/*
 * Broadcom bcm2836 SoC, as used in Raspberry Pi 2
 * 4 x Cortex-A7 processor (armv7)
 */
#include "mem.h"
#include "arm.h"
#include "arminstr.ha"

#define WFI	WORD	$0xe320f003	/* wait for interrupt */
#define WFI_EQ	WORD	$0x0320f003	/* wait for interrupt if eq */


TEXT armstart(SB), 1, $-4

	/*
	 * if not cpu0, go to secondary startup
	 */
	CPUID(R1)
	BNE	reset

	/*
	 * disable the mmu and caches
	 * invalidate tlb
	 */
	MRC	CpSC, 0, R1, C(CpCONTROL), C(0), CpMainctl
	BIC	$(CpCdcache|CpCicache|CpCmmu), R1
	ORR	$(CpCsbo|CpCsw), R1
	BIC	$CpCsbz, R1
	MCR	CpSC, 0, R1, C(CpCONTROL), C(0), CpMainctl
	MCR	CpSC, 0, R0, C(CpTLB), C(CpTLBinvu), CpTLBinv
	ISB

	/*
	 * clear mach and page tables
	 */
	MOVW	$PADDR(CPUADDR), R1
	MOVW	$PADDR(KTZERO), R2
_ramZ:
	MOVW	R0, (R1)
	ADD	$4, R1
	CMP	R1, R2
	BNE	_ramZ

	/*
	 * start stack at top of mach (physical addr)
	 * set up page tables for kernel
	 */
	MOVW	$PADDR(CPUADDR+CPUSIZE-4), R13
	MOVW	$PADDR(L1), R0
	BL	,mmuinit(SB)

	/*
	 * set up domain access control and page table base
	 */
	MOVW	$Client, R1
	MCR	CpSC, 0, R1, C(CpDAC), C(0)
	MOVW	$PADDR(L1), R1
	ORR		$(CpTTBs/*|CpTTBowba|CpTTBiwba*/), R1
	MCR	CpSC, 0, R1, C(CpTTB), C(0)
	MCR	CpSC, 0, R1, C(CpTTB), C(0), CpTTB1	/* cortex has two */

	/*
	 * invalidate my caches before enabling
	 */
	BL	cachedinv(SB)
	BL	cacheiinv(SB)
	BL	l2cacheuinv(SB)
	BARRIERS

	/*
	 * enable caches, mmu, and high vectors
	 */
	MRC	CpSC, 0, R1, C(CpCONTROL), C(0), CpAuxctl
	ORR	$CpACsmp, R1		/* turn SMP on */
	MCR	CpSC, 0, R1, C(CpCONTROL), C(0), CpAuxctl
	BARRIERS

	MRC	CpSC, 0, R0, C(CpCONTROL), C(0), CpMainctl
	ORR	$(CpChv|CpCdcache|CpCicache|CpCmmu), R0
	MCR	CpSC, 0, R0, C(CpCONTROL), C(0), CpMainctl
	BARRIERS

	/*
	 * switch SB, SP, and PC into KZERO space
	 */
	MOVW	$setR12(SB), R12
	MOVW	$(CPUADDR+CPUSIZE-4), R13
	MOVW	$_startpg(SB), R15

TEXT _startpg(SB), 1, $-4

	/*
	 * enable cycle counter
	 */
	MOVW	$(1<<31), R1
	MCR	CpSC, 0, R1, C(CpCLD), C(CpCLDena), CpCLDenacyc
	MOVW	$1, R1
	MCR	CpSC, 0, R1, C(CpCLD), C(CpCLDena), CpCLDenapmnc

	/*
	 * call main and loop forever if it returns
	 */
	BL	,main(SB)
	B	,0(PC)

	BL	_div(SB)		/* hack to load _div, etc. */

/*
 * startup entry for cpu(s) other than 0
 */
TEXT cpureset(SB), 1, $-4
reset:
	/*
	 * load physical base for SB addressing while mmu is off
	 * keep a handy zero in R0 until first function call
	 */
	MOVW	$setR12(SB), R12
	SUB	$KZERO, R12
	MOVW	$0, R0

	/*
	 * SVC mode, interrupts disabled
	 */
	MOVW	$(PsrDirq|PsrDfiq|PsrMsvc), R1
	MOVW	R1, CPSR

	/*
	 * disable the mmu and caches
	 * invalidate tlb
	 */
	MRC	CpSC, 0, R1, C(CpCONTROL), C(0), CpMainctl
	BIC	$(CpCdcache|CpCicache|CpCmmu), R1
	ORR	$(CpCsbo|CpCsw), R1
	BIC	$CpCsbz, R1
	MCR	CpSC, 0, R1, C(CpCONTROL), C(0), CpMainctl
	MCR	CpSC, 0, R0, C(CpTLB), C(CpTLBinvu), CpTLBinv
	ISB

	/*
	 * find Mach for this cpu
	 */
	MRC	CpSC, 0, R2, C(CpID), C(CpIDidct), CpIDmpid
	AND	$(MAXCPUS-1), R2	/* mask out non-cpu-id bits */
	SLL	$2, R2			/* convert to word index */
	MOVW	$cpus(SB), R0
	ADD	R2, R0			/* R0 = &cpus[cpuid] */
	MOVW	(R0), R0		/* R0 = cpus[cpuid] */
	CMP	$0, R0
	MOVW.EQ	$CPUADDR, R0		/* paranoia: use CPUADDR if 0 */
	SUB	$KZERO, R0		/* phys addr */
	MOVW	R0, R(MACH)		/* m = PADDR(cpus[cpuid]) */

	/*
	 * start stack at top of local Mach
	 */
	MOVW	R(MACH), R13
	ADD		$(CPUSIZE-4), R13

	/*
	 * set up page tables for kernel
	 */
	MOVW	12(R(MACH)), R0	/* m->mmul1 */
	SUB	$KZERO, R0		/* phys addr */
	BL	mmuinit(SB)

	/*
	 * set up domain access control and page table base
	 */
	MOVW	$Client, R1
	MCR	CpSC, 0, R1, C(CpDAC), C(0)
	MOVW	12(R(MACH)), R1	/* m->mmul1 */
	SUB	$KZERO, R1		/* phys addr */
	ORR		$(CpTTBs/*|CpTTBowba|CpTTBiwba*/), R1
	MCR	CpSC, 0, R1, C(CpTTB), C(0)
	MCR	CpSC, 0, R1, C(CpTTB), C(0), CpTTB1	/* cortex has two */

	/*
	 * invalidate my caches before enabling
	 */
	BL	cachedinv(SB)
	BL	cacheiinv(SB)
	BARRIERS

	/*
	 * enable caches, mmu, and high vectors
	 */
	MRC	CpSC, 0, R1, C(CpCONTROL), C(0), CpAuxctl
	ORR	$CpACsmp, R1		/* turn SMP on */
	MCR	CpSC, 0, R1, C(CpCONTROL), C(0), CpAuxctl
	BARRIERS

	MRC	CpSC, 0, R0, C(CpCONTROL), C(0), CpMainctl
	ORR	$(CpChv|CpCdcache|CpCicache|CpCmmu), R0
	MCR	CpSC, 0, R0, C(CpCONTROL), C(0), CpMainctl
	BARRIERS

	/*
	 * switch MACH, SB, SP, and PC into KZERO space
	 */
	ADD	$KZERO, R(MACH)
	MOVW	$setR12(SB), R12
	ADD	$KZERO, R13
	MOVW	$_startpg2(SB), R15

TEXT _startpg2(SB), 1, $-4

	/*
	 * enable cycle counter
	 */
	MOVW	$(1<<31), R1
	MCR	CpSC, 0, R1, C(CpCLD), C(CpCLDena), CpCLDenacyc
	MOVW	$1, R1
	MCR	CpSC, 0, R1, C(CpCLD), C(CpCLDena), CpCLDenapmnc

	/*
	 * call cpustart and loop forever if it returns
	 */
	MRC	CpSC, 0, R0, C(CpID), C(CpIDidct), CpIDmpid
	AND	$(MAXCPUS-1), R0			/* mask out non-cpu-id bits */
	BL	,cpustart(SB)
	B	,0(PC)



        
TEXT cpctget(SB), 1, $-4			/* cache type */
	MRC	CpSC, 0, R0, C(CpID), C(CpIDidct), CpIDct
	RET



TEXT arch_idlehands(SB), $-4
	MOVW	CPSR, R3
	ORR	$(PsrDirq|PsrDfiq), R3, R1		/* splfhi */
	MOVW	R1, CPSR

	DSB
	MOVW	nrdy(SB), R0
	CMP	$0, R0
/*** with WFI, local timer interrupts can be lost and dispatching stops
	WFI_EQ
***/
	DSB

	MOVW	R3, CPSR			/* splx */
	RET
