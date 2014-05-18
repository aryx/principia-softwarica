/*s: dat_core.h */

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
    /*s: [[ArchMach]] cpuid fields */
    char  cpuidid[16];
    char* cpuidtype;
    int cpuidax;
    int cpuiddx;
    /*e: [[ArchMach]] cpuid fields */
    /*s: [[ArchMach]] other fields */
    Proc* externup;   /* extern register Proc *up */
    /*x: [[ArchMach]] other fields */
    Segdesc *gdt;     /* gdt for this processor */
    /*x: [[ArchMach]] other fields */
    // TODO: have a ArchMachMMU like in bcm/
    kern_addr2  pdb;      /* page directory base for this processor (va) */
    Page* pdbpool;
    int pdbcnt;
    /*x: [[ArchMach]] other fields */
    Tss*  tss;      /* tss for this processor */
    /*x: [[ArchMach]] other fields */
    int havepge;
    /*x: [[ArchMach]] other fields */
    // for perfticks, tsc = time stamp counter
    bool havetsc;
    /*x: [[ArchMach]] other fields */
    int pdballoc;
    int pdbfree;
    /*x: [[ArchMach]] other fields */
    int loopconst;
    /*x: [[ArchMach]] other fields */
    ArchFPsave *fpsavalign;
    /*x: [[ArchMach]] other fields */
    Lock  apictimerlock;
    /*x: [[ArchMach]] other fields */
    uvlong tscticks;
    /*x: [[ArchMach]] other fields */
    vlong mtrrcap;
    vlong mtrrdef;
    vlong mtrrfix[11];
    vlong mtrrvar[32];    /* 256 max. */
    /*e: [[ArchMach]] other fields */
};
/*e: struct ArchMach */

//*****************************************************************************
// Conf extension?
//*****************************************************************************

/*e: dat_core.h */
