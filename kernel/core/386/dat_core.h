/*s: dat_core.h */

//*****************************************************************************
// Cpu extension
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
    ulong d0; // ??
    ulong d1; // ??
};
/*e: struct Segdesc */

/*s: struct ArchCpu */
struct ArchCpu {
    /*s: [[Cpu]] [[Arch]] cpuid fields */
    char  cpuidid[16];
    char* cpuidtype;
    int cpuidax;
    int cpuiddx;
    /*e: [[Cpu]] [[Arch]] cpuid fields */
    /*s: [[Cpu]] [[Arch]] other fields */
    // array<Segdesc>
    Segdesc *gdt;     /* gdt for this processor */
    /*x: [[Cpu]] [[Arch]] other fields */
    kern_addr2   pdproto;      /* page directory base for this processor (va) */
    /*x: [[Cpu]] [[Arch]] other fields */
    Page* mmupdpool;
    int mmupdcnt;
    /*x: [[Cpu]] [[Arch]] other fields */
    Tss*  tss;      /* tss for this processor */
    /*x: [[Cpu]] [[Arch]] other fields */
    int havepge;
    /*x: [[Cpu]] [[Arch]] other fields */
    // for perfticks, tsc = time stamp counter
    bool havetsc;
    /*x: [[Cpu]] [[Arch]] other fields */
    int mmupdalloc;
    int mmupdfree;
    /*x: [[Cpu]] [[Arch]] other fields */
    int loopconst;
    /*x: [[Cpu]] [[Arch]] other fields */
    ArchFPsave *fpsavalign;
    /*x: [[Cpu]] [[Arch]] other fields */
    Lock  apictimerlock;
    /*x: [[Cpu]] [[Arch]] other fields */
    uvlong tscticks;
    /*x: [[Cpu]] [[Arch]] other fields */
    vlong mtrrcap;
    vlong mtrrdef;
    vlong mtrrfix[11];
    vlong mtrrvar[32];    /* 256 max. */
    /*e: [[Cpu]] [[Arch]] other fields */
};
/*e: struct ArchCpu */

//*****************************************************************************
// Conf extension?
//*****************************************************************************

/*e: dat_core.h */
