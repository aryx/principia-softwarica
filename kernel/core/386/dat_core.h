/*s: dat_core.h */

//*****************************************************************************
// Conf extension
//*****************************************************************************

/*s: struct ArchConf */
struct ArchConf {
    ulong base0;    /* base of bank 0 */
    ulong base1;    /* base of bank 1 */
};
/*e: struct ArchConf */

//*****************************************************************************
// Mach extension
//*****************************************************************************

/*s: struct Tss */
struct Tss {
    ulong link;     /* link (old TSS selector) */
    ulong esp0;     /* privilege level 0 stack pointer */
    ulong ss0;      /* privilege level 0 stack selector */
    ulong esp1;     /* privilege level 1 stack pointer */
    ulong ss1;      /* privilege level 1 stack selector */
    ulong esp2;     /* privilege level 2 stack pointer */
    ulong ss2;      /* privilege level 2 stack selector */
    ulong xcr3;     /* page directory base register - not used because we don't use trap gates */
    ulong eip;      /* instruction pointer */
    ulong eflags;     /* flags register */
    ulong eax;      /* general registers */
    ulong   ecx;
    ulong edx;
    ulong ebx;
    ulong esp;
    ulong ebp;
    ulong esi;
    ulong edi;
    ulong es;     /* segment selectors */
    ulong cs;
    ulong ss;
    ulong ds;
    ulong fs;
    ulong gs;
    ulong ldt;      /* selector for task's LDT */
    ulong iomap;      /* I/O map base address + T-bit */
};
/*e: struct Tss */

/*s: struct Segdesc */
struct Segdesc
{
    ulong d0;
    ulong d1;
};
/*e: struct Segdesc */

/*s: struct ArchMach */
struct ArchMach {
    /*s: ArchMach other fields */
    Proc* externup;   /* extern register Proc *up */
    /*x: ArchMach other fields */
    // TODO: have a ArchMachMMU like in bcm/
    ulong*  pdb;      /* page directory base for this processor (va) */
    Tss*  tss;      /* tss for this processor */
    Segdesc *gdt;     /* gdt for this processor */
    Page* pdbpool;
    int pdbcnt;
    //  int inclockintr;

    Lock  apictimerlock;
    ArchFPsave *fpsavalign;

    // for perfticks, tsc = time stamp counter
    bool havetsc;

    int loopconst;
    int cpuidax;
    int cpuiddx;
    char  cpuidid[16];
    char* cpuidtype;
    int havepge;
    uvlong tscticks;
    int pdballoc;
    int pdbfree;
    vlong mtrrcap;
    vlong mtrrdef;
    vlong mtrrfix[11];
    vlong mtrrvar[32];    /* 256 max. */
    /*e: ArchMach other fields */
};
/*e: struct ArchMach */
/*e: dat_core.h */
