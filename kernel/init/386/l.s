/*s: l.s */
#include "mem.h"
#undef DELAY

//*****************************************************************************
// Constants/Macros
//*****************************************************************************

#define PADDR(ka)        ((ka) & ~KZERO)
#define KADDR(pa)        (KZERO|(pa))

/*
 * Some machine instructions not handled by 8[al].
 */
#define DELAY           BYTE $0xEB; BYTE $0x00  /* JMP .+2 */

/*
 * Macros for calculating offsets within the page directory base
 * and page tables. Note that these are assembler-specific hence
 * the '<<2'.
 */
#define PDO(a)          (((((a))>>22) & 0x03FF)<<2)
#define PTO(a)          (((((a))>>12) & 0x03FF)<<2)
// <<2 because each PDE or PTE is 4 bytes.

//*****************************************************************************
// Entry point!! (after jump from l_multiboot.s)
//*****************************************************************************

/*s: function _setup_segmentation */
TEXT _setup_segmentation(SB), $0
        CLI                     /* make sure interrupts are off */

        /* set up the gdt so we have sane plan 9 style gdts. */
        MOVL    $tgdtptr(SB), AX
        ANDL    $~KZERO, AX
        MOVL    (AX), GDTR
        MOVW    $1, AX
        MOVW    AX, MSW

        /* clear prefetch queue (weird code to avoid optimizations) */
        DELAY

        /* set segs to something sane (avoid traps later) */
        MOVW    $(1<<3), AX
        MOVW    AX, DS
        MOVW    AX, SS
        MOVW    AX, ES
        MOVW    AX, FS
        MOVW    AX, GS

/*      JMP     $(2<<3):$_setup_pagination(SB) /**/
         BYTE   $0xEA
         LONG   $_setup_pagination-KZERO(SB)
         WORD   $(2<<3)
/*e: function _setup_segmentation */

//*****************************************************************************
// Gdts Data
//*****************************************************************************

/*s: global tgdt */
/*
 *  gdt to get us to 32-bit/segmented/unpaged mode
 */
TEXT tgdt(SB), $0

        /* null descriptor */
        LONG    $0
        LONG    $0

        /* data segment descriptor for 4 gigabytes (PL 0) */
        LONG    $(0xFFFF)
        LONG    $(SEGG|SEGB|(0xF<<16)|SEGP|SEGPL(0)|SEGDATA|SEGW)

        /* exec segment descriptor for 4 gigabytes (PL 0) */
        LONG    $(0xFFFF)
        LONG    $(SEGG|SEGD|(0xF<<16)|SEGP|SEGPL(0)|SEGEXEC|SEGR)
/*e: global tgdt */

/*s: global tgdtptr */
/*
 *  pointer to initial gdt
 *  Note the -KZERO which puts the physical address in the gdtptr. 
 *  that's needed as we start executing in physical addresses. 
 */
TEXT tgdtptr(SB), $0
        WORD    $(3*8)
        LONG    $tgdt-KZERO(SB)
/*e: global tgdtptr */

// realmode stuff
TEXT m0rgdtptr(SB), $0
        WORD    $(NGDT*8-1)
        LONG    $(CPU0GDT-KZERO)

TEXT m0gdtptr(SB), $0
        WORD    $(NGDT*8-1)
        LONG    $CPU0GDT

TEXT m0idtptr(SB), $0
        WORD $(256*8-1)
        LONG $IDTADDR

//*****************************************************************************
// Assume protected 32 bit and GTD done
//*****************************************************************************

/*s: function _setup_pagination */
/*
 * In protected mode with paging turned off and segment registers setup
 * to linear map all memory.
 * Make the basic page tables for processor 0. Five pages are needed for
 * the basic set:
 *      a page directory;
 *      page table for mapping the first 4MB of physical memory to KZERO;
 *      a page for the GDT;
 *      virtual and physical pages for mapping the Cpu structure.
 * The remaining PTEs will be allocated later when memory is sized.
 *
 * An identity mmu map is also needed for the switch to virtual mode.
 * This identity mapping is removed once the MMU is going and the JMP has
 * been made to virtual memory.
 */
TEXT _setup_pagination(SB), $0
        /* At this point, the GDT setup is done. */

        MOVL    $PADDR(CPU0PD), DI             /* clear 4 pages for the tables etc. */
        XORL    AX, AX
        MOVL    $(4*BY2PG), CX
        SHRL    $2, CX

        CLD
        REP;    STOSL

        MOVL    $PADDR(CPU0PD), AX
        ADDL    $PDO(KZERO), AX                 /* page directory offset for KZERO */
        MOVL    $PADDR(CPU0PT), (AX)           /* PTE's for KZERO */
        MOVL    $(PTEWRITE|PTEVALID), BX        /* page permissions */
        ORL     BX, (AX)


        MOVL    $PADDR(CPU0PT), AX             /* first page of page table */
        MOVL    $1024, CX                       /* 1024 pages in 4MB */
_setpte:
        MOVL    BX, (AX)
        ADDL    $(1<<PGSHIFT), BX
        ADDL    $4, AX
        LOOP    _setpte


        MOVL    $PADDR(CPU0PT), AX
        ADDL    $PTO(CPUADDR), AX              /* page table entry offset for CPUADDR */
        MOVL    $PADDR(CPU0CPU), (AX)          /* PTE for Cpu */
        MOVL    $(PTEWRITE|PTEVALID), BX        /* page permissions */
        ORL     BX, (AX)

/*
 * Now ready to use the new map. Make sure the processor options are what is wanted.
 * It is necessary on some processors to immediately follow mode switching with a JMP instruction
 * to clear the prefetch queues.
 */
        MOVL    $PADDR(CPU0PD), CX             /* load address of page directory */
        MOVL    (PDO(KZERO))(CX), DX            /* double-map KZERO at 0 */
        MOVL    DX, (PDO(0))(CX)

        MOVL    CX, CR3
        DELAY                                   /* JMP .+2 */

        MOVL    CR0, DX
        ORL     $0x80010000, DX                 /* PG|WP */
        ANDL    $~0x6000000A, DX                /* ~(CD|NW|TS|MP) */

        MOVL    $_setup_bss_stack(SB), AX       /* this is a virtual address */
        MOVL    DX, CR0                         /* turn on paging */
        JMP*    AX                              /* jump to the virtual nirvana */
/*e: function _setup_pagination */

/*s: function _setup_bss_stack */
/*
 * Basic machine environment set, can clear BSS and create a stack.
 * The stack starts at the top of the page containing the Cpu structure.
 * The x86 architecture forces the use of the same virtual address for
 * each processor's Cpu structure, so the global Cpu pointer 'cpu' can
 * be initialised here.
 */
TEXT _setup_bss_stack(SB), $0
        MOVL    $0, (PDO(0))(CX)                /* undo double-map of KZERO at 0 */
        MOVL    CX, CR3                         /* load and flush the mmu */

_clearbss:
        MOVL    $edata(SB), DI
        XORL    AX, AX
        MOVL    $end(SB), CX
        SUBL    DI, CX                          /* end-edata bytes */
        SHRL    $2, CX                          /* end-edata doublewords */

        CLD
        REP;    STOSL                           /* clear BSS */

        MOVL    $CPUADDR, SP
        MOVL    SP, cpu(SB)                /* initialise global Cpu pointer */
        MOVL    $0, 0(SP)                       /* initialise cpu->cpuno */


        ADDL    $(CPUSIZE-4), SP               /* initialise stack */

/*s: end of _setup_bss_stack */
/*
 * Need to do one final thing to ensure a clean machine environment,
 * clear the EFLAGS register, which can only be done once there is a stack.
 */
        MOVL    $0, AX
        PUSHL   AX
        POPFL

        CALL    main(SB)
/*e: end of _setup_bss_stack */
/*e: function _setup_bss_stack */

        
//*****************************************************************************
// CPU registers accessor
//*****************************************************************************
               
/*
 * Read/write various system registers.
 * CR4 and the 'model specific registers' should only be read/written
 * after it has been determined the processor supports them
 */
TEXT lgdt(SB), $0                               /* GDTR - global descriptor table */
        MOVL    gdtptr+0(FP), AX
        MOVL    (AX), GDTR
        RET

TEXT lidt(SB), $0                               /* IDTR - interrupt descriptor table */
        MOVL    idtptr+0(FP), AX
        MOVL    (AX), IDTR
        RET

TEXT ltr(SB), $0                                /* TR - task register */
        MOVL    tptr+0(FP), AX
        MOVW    AX, TASK
        RET



TEXT getcr0(SB), $0                             /* CR0 - processor control */
        MOVL    CR0, AX
        RET

TEXT getcr2(SB), $0                             /* CR2 - page fault linear address */
        MOVL    CR2, AX
        RET

TEXT getcr3(SB), $0                             /* CR3 - page directory base */
        MOVL    CR3, AX
        RET

TEXT getcr4(SB), $0                             /* CR4 - extensions */
        MOVL    CR4, AX
        RET


TEXT putcr0(SB), $0
        MOVL    cr0+0(FP), AX
        MOVL    AX, CR0
        RET

TEXT putcr3(SB), $0
        MOVL    cr3+0(FP), AX
        MOVL    AX, CR3
        RET

TEXT putcr4(SB), $0
        MOVL    cr4+0(FP), AX
        MOVL    AX, CR4
        RET

/*e: l.s */
