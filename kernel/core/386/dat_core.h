/*s: dat_core.h */

//*****************************************************************************
// Cpu extension
//*****************************************************************************

/*s: struct Tss(x86) */
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
/*e: struct Tss(x86) */

/*s: struct Segdesc(x86) */
struct Segdesc
{
    ulong d0; // ??
    ulong d1; // ??
};
/*e: struct Segdesc(x86) */

/*s: struct ArchCpu(x86) */
struct ArchCpu {
    /*s: [[Cpu]] [[Arch]] cpuid fields(x86) */
    char  cpuidid[16];
    char* cpuidtype;
    int cpuidax;
    int cpuiddx;
    /*e: [[Cpu]] [[Arch]] cpuid fields(x86) */
    /*s: [[Cpu]] [[Arch]] other fields(x86) */
    // array<Segdesc>
    Segdesc *gdt;     /* gdt for this processor */
    /*x: [[Cpu]] [[Arch]] other fields(x86) */
    kern_addr2   pdproto;      /* page directory base for this processor (va) */
    /*x: [[Cpu]] [[Arch]] other fields(x86) */
    Page* mmupdpool;
    int mmupdcnt;
    /*x: [[Cpu]] [[Arch]] other fields(x86) */
    Tss*  tss;      /* tss for this processor */
    /*x: [[Cpu]] [[Arch]] other fields(x86) */
    int havepge;
    /*x: [[Cpu]] [[Arch]] other fields(x86) */
    // for perfticks, tsc = time stamp counter
    bool havetsc;
    /*x: [[Cpu]] [[Arch]] other fields(x86) */
    ArchFPsave *fpsavalign;
    /*x: [[Cpu]] [[Arch]] other fields(x86) */
    int mmupdalloc;
    int mmupdfree;
    /*x: [[Cpu]] [[Arch]] other fields(x86) */
    int loopconst;
    /*x: [[Cpu]] [[Arch]] other fields(x86) */
    vlong mtrrcap;
    vlong mtrrdef;
    vlong mtrrfix[11];
    vlong mtrrvar[32];    /* 256 max. */
    /*x: [[Cpu]] [[Arch]] other fields(x86) */
    Lock  apictimerlock;
    /*x: [[Cpu]] [[Arch]] other fields(x86) */
    uvlong tscticks;
    /*e: [[Cpu]] [[Arch]] other fields(x86) */
};
/*e: struct ArchCpu(x86) */

//*****************************************************************************
// Conf extension?
//*****************************************************************************

/*e: dat_core.h */
