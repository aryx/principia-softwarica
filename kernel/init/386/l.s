#include "mem.h"
#undef DELAY

#define PADDR(a)	((a) & ~KZERO)
#define KADDR(a)	(KZERO|(a))

/*
 * Some machine instructions not handled by 8[al].
 */
#define DELAY		BYTE $0xEB; BYTE $0x00	/* JMP .+2 */
#define CPUID		BYTE $0x0F; BYTE $0xA2	/* CPUID, argument in AX */
#define WRMSR		BYTE $0x0F; BYTE $0x30	/* WRMSR, argument in AX/DX (lo/hi) */
#define RDTSC 		BYTE $0x0F; BYTE $0x31	/* RDTSC, result in AX/DX (lo/hi) */
#define RDMSR		BYTE $0x0F; BYTE $0x32	/* RDMSR, result in AX/DX (lo/hi) */
#define INVLPG	BYTE $0x0F; BYTE $0x01; BYTE $0x39	/* INVLPG (%ecx) */
#define WBINVD	BYTE $0x0F; BYTE $0x09

/*
 * Macros for calculating offsets within the page directory base
 * and page tables. Note that these are assembler-specific hence
 * the '<<2'.
 */
#define PDO(a)		(((((a))>>22) & 0x03FF)<<2)
#define PTO(a)		(((((a))>>12) & 0x03FF)<<2)


/*
 * In protected mode with paging turned off and segment registers setup
 * to linear map all memory. Entered via a jump to PADDR(entry),
 * the physical address of the virtual kernel entry point of KADDR(entry).
 * Make the basic page tables for processor 0. Six pages are needed for
 * the basic set:
 *	a page directory;
 *	page tables for mapping the first 8MB of physical memory to KZERO;
 *	a page for the GDT;
 *	virtual and physical pages for mapping the Mach structure.
 * The remaining PTEs will be allocated later when memory is sized.
 * An identity mmu map is also needed for the switch to virtual mode.
 * This identity mapping is removed once the MMU is going and the JMP has
 * been made to virtual memory.
 */
TEXT _startPADDR(SB), $0
	CLI					/* make sure interrupts are off */

	/* set up the gdt so we have sane plan 9 style gdts. */
	MOVL	$tgdtptr(SB), AX
	ANDL	$~KZERO, AX
	MOVL	(AX), GDTR
	MOVW	$1, AX
	MOVW	AX, MSW

	/* clear prefetch queue (weird code to avoid optimizations) */
	DELAY

	/* set segs to something sane (avoid traps later) */
	MOVW	$(1<<3), AX
	MOVW	AX, DS
	MOVW	AX, SS
	MOVW	AX, ES
	MOVW	AX, FS
	MOVW	AX, GS

/*	JMP	$(2<<3):$mode32bit(SB) /**/
	 BYTE	$0xEA
	 LONG	$mode32bit-KZERO(SB)
	 WORD	$(2<<3)

/*
 *  gdt to get us to 32-bit/segmented/unpaged mode
 */
TEXT tgdt(SB), $0

	/* null descriptor */
	LONG	$0
	LONG	$0

	/* data segment descriptor for 4 gigabytes (PL 0) */
	LONG	$(0xFFFF)
	LONG	$(SEGG|SEGB|(0xF<<16)|SEGP|SEGPL(0)|SEGDATA|SEGW)

	/* exec segment descriptor for 4 gigabytes (PL 0) */
	LONG	$(0xFFFF)
	LONG	$(SEGG|SEGD|(0xF<<16)|SEGP|SEGPL(0)|SEGEXEC|SEGR)

/*
 *  pointer to initial gdt
 *  Note the -KZERO which puts the physical address in the gdtptr. 
 *  that's needed as we start executing in physical addresses. 
 */
TEXT tgdtptr(SB), $0
	WORD	$(3*8)
	LONG	$tgdt-KZERO(SB)

TEXT m0rgdtptr(SB), $0
	WORD	$(NGDT*8-1)
	LONG	$(CPU0GDT-KZERO)

TEXT m0gdtptr(SB), $0
	WORD	$(NGDT*8-1)
	LONG	$CPU0GDT

TEXT m0idtptr(SB), $0
	WORD $(256*8-1)
	LONG $IDTADDR

TEXT mode32bit(SB), $0
	/* At this point, the GDT setup is done. */

	MOVL	$PADDR(CPU0PDB), DI		/* clear 4 pages for the tables etc. */
	XORL	AX, AX
	MOVL	$(4*BY2PG), CX
	SHRL	$2, CX

	CLD
	REP;	STOSL

	MOVL	$PADDR(CPU0PDB), AX
	ADDL	$PDO(KZERO), AX			/* page directory offset for KZERO */
	MOVL	$PADDR(CPU0PTE), (AX)		/* PTE's for KZERO */
	MOVL	$(PTEWRITE|PTEVALID), BX	/* page permissions */
	ORL	BX, (AX)

	ADDL	$4, AX
	MOVL	$PADDR(CPU0PTE1), (AX)		/* PTE's for KZERO+4MB */
	MOVL	$(PTEWRITE|PTEVALID), BX	/* page permissions */
	ORL	BX, (AX)

	MOVL	$PADDR(CPU0PTE), AX		/* first page of page table */
	MOVL	$1024, CX			/* 1024 pages in 4MB */
_setpte:
	MOVL	BX, (AX)
	ADDL	$(1<<PGSHIFT), BX
	ADDL	$4, AX
	LOOP	_setpte

	MOVL	$PADDR(CPU0PTE1), AX		/* second page of page table */
	MOVL	$1024, CX			/* 1024 pages in 4MB */
_setpte1:
	MOVL	BX, (AX)
	ADDL	$(1<<PGSHIFT), BX
	ADDL	$4, AX
	LOOP	_setpte1

	MOVL	$PADDR(CPU0PTE), AX
	ADDL	$PTO(MACHADDR), AX		/* page table entry offset for MACHADDR */
	MOVL	$PADDR(CPU0MACH), (AX)		/* PTE for Mach */
	MOVL	$(PTEWRITE|PTEVALID), BX	/* page permissions */
	ORL	BX, (AX)

/*
 * Now ready to use the new map. Make sure the processor options are what is wanted.
 * It is necessary on some processors to immediately follow mode switching with a JMP instruction
 * to clear the prefetch queues.
 */
	MOVL	$PADDR(CPU0PDB), CX		/* load address of page directory */
	MOVL	(PDO(KZERO))(CX), DX		/* double-map KZERO at 0 */
	MOVL	DX, (PDO(0))(CX)
	MOVL	CX, CR3
	DELAY					/* JMP .+2 */

	MOVL	CR0, DX
	ORL	$0x80010000, DX			/* PG|WP */
	ANDL	$~0x6000000A, DX		/* ~(CD|NW|TS|MP) */

	MOVL	$_startpg(SB), AX		/* this is a virtual address */
	MOVL	DX, CR0				/* turn on paging */
	JMP*	AX				/* jump to the virtual nirvana */

/*
 * Basic machine environment set, can clear BSS and create a stack.
 * The stack starts at the top of the page containing the Mach structure.
 * The x86 architecture forces the use of the same virtual address for
 * each processor's Mach structure, so the global Mach pointer 'm' can
 * be initialised here.
 */
TEXT _startpg(SB), $0
	MOVL	$0, (PDO(0))(CX)		/* undo double-map of KZERO at 0 */
	MOVL	CX, CR3				/* load and flush the mmu */

_clearbss:
	MOVL	$edata(SB), DI
	XORL	AX, AX
	MOVL	$end(SB), CX
	SUBL	DI, CX				/* end-edata bytes */
	SHRL	$2, CX				/* end-edata doublewords */

	CLD
	REP;	STOSL				/* clear BSS */

	MOVL	$MACHADDR, SP
	MOVL	SP, m(SB)			/* initialise global Mach pointer */
	MOVL	$0, 0(SP)			/* initialise m->machno */


	ADDL	$(MACHSIZE-4), SP		/* initialise stack */

/*
 * Need to do one final thing to ensure a clean machine environment,
 * clear the EFLAGS register, which can only be done once there is a stack.
 */
	MOVL	$0, AX
	PUSHL	AX
	POPFL

	CALL	main(SB)

/*
 * Save registers.
 */
TEXT saveregs(SB), $0
	/* appease 8l */
	SUBL $32, SP
	POPL AX
	POPL AX
	POPL AX
	POPL AX
	POPL AX
	POPL AX
	POPL AX
	POPL AX
	
	PUSHL	AX
	PUSHL	BX
	PUSHL	CX
	PUSHL	DX
	PUSHL	BP
	PUSHL	DI
	PUSHL	SI
	PUSHFL

	XCHGL	32(SP), AX	/* swap return PC and saved flags */
	XCHGL	0(SP), AX
	XCHGL	32(SP), AX
	RET

TEXT restoreregs(SB), $0
	/* appease 8l */
	PUSHL	AX
	PUSHL	AX
	PUSHL	AX
	PUSHL	AX
	PUSHL	AX
	PUSHL	AX
	PUSHL	AX
	PUSHL	AX
	ADDL	$32, SP
	
	XCHGL	32(SP), AX	/* swap return PC and saved flags */
	XCHGL	0(SP), AX
	XCHGL	32(SP), AX

	POPFL
	POPL	SI
	POPL	DI
	POPL	BP
	POPL	DX
	POPL	CX
	POPL	BX
	POPL	AX
	RET

       
/*
 * Read/write various system registers.
 * CR4 and the 'model specific registers' should only be read/written
 * after it has been determined the processor supports them
 */
TEXT lgdt(SB), $0				/* GDTR - global descriptor table */
	MOVL	gdtptr+0(FP), AX
	MOVL	(AX), GDTR
	RET

TEXT lidt(SB), $0				/* IDTR - interrupt descriptor table */
	MOVL	idtptr+0(FP), AX
	MOVL	(AX), IDTR
	RET

TEXT ltr(SB), $0				/* TR - task register */
	MOVL	tptr+0(FP), AX
	MOVW	AX, TASK
	RET

TEXT getcr0(SB), $0				/* CR0 - processor control */
	MOVL	CR0, AX
	RET

TEXT getcr2(SB), $0				/* CR2 - page fault linear address */
	MOVL	CR2, AX
	RET

TEXT getcr3(SB), $0				/* CR3 - page directory base */
	MOVL	CR3, AX
	RET

TEXT putcr0(SB), $0
	MOVL	cr0+0(FP), AX
	MOVL	AX, CR0
	RET

TEXT putcr3(SB), $0
	MOVL	cr3+0(FP), AX
	MOVL	AX, CR3
	RET

TEXT getcr4(SB), $0				/* CR4 - extensions */
	MOVL	CR4, AX
	RET

TEXT putcr4(SB), $0
	MOVL	cr4+0(FP), AX
	MOVL	AX, CR4
	RET

TEXT invlpg(SB), $0
	/* 486+ only */
	MOVL	va+0(FP), CX
	INVLPG
	RET

TEXT wbinvd(SB), $0
	WBINVD
	RET

TEXT _cycles(SB), $0				/* time stamp counter */
	RDTSC
	MOVL	vlong+0(FP), CX			/* &vlong */
	MOVL	AX, 0(CX)			/* lo */
	MOVL	DX, 4(CX)			/* hi */
	RET

/*
 * stub for:
 * time stamp counter; low-order 32 bits of 64-bit cycle counter
 * Runs at fasthz/4 cycles per second (m->clkin>>3)
 */
TEXT lcycles(SB),1,$0
	RDTSC
	RET

TEXT rdmsr(SB), $0				/* model-specific register */
	MOVL	index+0(FP), CX
	RDMSR
	MOVL	vlong+4(FP), CX			/* &vlong */
	MOVL	AX, 0(CX)			/* lo */
	MOVL	DX, 4(CX)			/* hi */
	RET
	
TEXT wrmsr(SB), $0
	MOVL	index+0(FP), CX
	MOVL	lo+4(FP), AX
	MOVL	hi+8(FP), DX
	WRMSR
	RET

/*
 * Try to determine the CPU type which requires fiddling with EFLAGS.
 * If the Id bit can be toggled then the CPUID instruction can be used
 * to determine CPU identity and features. First have to check if it's
 * a 386 (Ac bit can't be set). If it's not a 386 and the Id bit can't be
 * toggled then it's an older 486 of some kind.
 *
 *	cpuid(fun, regs[4]);
 */
TEXT cpuid(SB), $0
	MOVL	$0x240000, AX
	PUSHL	AX
	POPFL					/* set Id|Ac */
	PUSHFL
	POPL	BX				/* retrieve value */
	MOVL	$0, AX
	PUSHL	AX
	POPFL					/* clear Id|Ac, EFLAGS initialised */
	PUSHFL
	POPL	AX				/* retrieve value */
	XORL	BX, AX
	TESTL	$0x040000, AX			/* Ac */
	JZ	_cpu386				/* can't set this bit on 386 */
	TESTL	$0x200000, AX			/* Id */
	JZ	_cpu486				/* can't toggle this bit on some 486 */
	/* load registers */
	MOVL	regs+4(FP), BP
	MOVL	fn+0(FP), AX			/* cpuid function */
	MOVL	4(BP), BX
	MOVL	8(BP), CX			/* typically an index */
	MOVL	12(BP), DX
	CPUID
	JMP	_cpuid
_cpu486:
	MOVL	$0x400, AX
	JMP	_maybezapax
_cpu386:
	MOVL	$0x300, AX
_maybezapax:
	CMPL	fn+0(FP), $1
	JE	_zaprest
	XORL	AX, AX
_zaprest:
	XORL	BX, BX
	XORL	CX, CX
	XORL	DX, DX
_cpuid:
	MOVL	regs+4(FP), BP
	MOVL	AX, 0(BP)
	MOVL	BX, 4(BP)
	MOVL	CX, 8(BP)
	MOVL	DX, 12(BP)
	RET




TEXT mb386(SB), $0
	POPL	AX				/* return PC */
	PUSHFL
	PUSHL	CS
	PUSHL	AX
	IRETL

TEXT mb586(SB), $0
	XORL	AX, AX
	CPUID
	RET

TEXT sfence(SB), $0
	BYTE $0x0f
	BYTE $0xae
	BYTE $0xf8
	RET

TEXT lfence(SB), $0
	BYTE $0x0f
	BYTE $0xae
	BYTE $0xe8
	RET

TEXT mfence(SB), $0
	BYTE $0x0f
	BYTE $0xae
	BYTE $0xf0
	RET

//TEXT xchgw(SB), $0
//	MOVL	v+4(FP), AX
//	MOVL	p+0(FP), BX
//	XCHGW	AX, (BX)
//	RET

TEXT cmpswap486(SB), $0
	MOVL	addr+0(FP), BX
	MOVL	old+4(FP), AX
	MOVL	new+8(FP), CX
	LOCK
	BYTE $0x0F; BYTE $0xB1; BYTE $0x0B	/* CMPXCHGL CX, (BX) */
	JNZ didnt
	MOVL	$1, AX
	RET
didnt:
	XORL	AX,AX
	RET

TEXT mul64fract(SB), $0
/*
 * Multiply two 64-bit number s and keep the middle 64 bits from the 128-bit result
 * See ../port/tod.c for motivation.
 */
	MOVL	r+0(FP), CX
	XORL	BX, BX				/* BX = 0 */

	MOVL	a+8(FP), AX
	MULL	b+16(FP)			/* a1*b1 */
	MOVL	AX, 4(CX)			/* r2 = lo(a1*b1) */

	MOVL	a+8(FP), AX
	MULL	b+12(FP)			/* a1*b0 */
	MOVL	AX, 0(CX)			/* r1 = lo(a1*b0) */
	ADDL	DX, 4(CX)			/* r2 += hi(a1*b0) */

	MOVL	a+4(FP), AX
	MULL	b+16(FP)			/* a0*b1 */
	ADDL	AX, 0(CX)			/* r1 += lo(a0*b1) */
	ADCL	DX, 4(CX)			/* r2 += hi(a0*b1) + carry */

	MOVL	a+4(FP), AX
	MULL	b+12(FP)			/* a0*b0 */
	ADDL	DX, 0(CX)			/* r1 += hi(a0*b0) */
	ADCL	BX, 4(CX)			/* r2 += carry */
	RET


        
/*
 *  label consists of a stack pointer and a PC
 */
TEXT gotolabel(SB), $0
	MOVL	label+0(FP), AX
	MOVL	0(AX), SP			/* restore sp */
	MOVL	4(AX), AX			/* put return pc on the stack */
	MOVL	AX, 0(SP)
	MOVL	$1, AX				/* return 1 */
	RET

TEXT setlabel(SB), $0
	MOVL	label+0(FP), AX
	MOVL	SP, 0(AX)			/* store sp */
	MOVL	0(SP), BX			/* store return pc */
	MOVL	BX, 4(AX)
	MOVL	$0, AX				/* return 0 */
	RET

