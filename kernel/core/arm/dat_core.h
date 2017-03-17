/*s: core/arm/dat_core.h */

/*s: type PTE(arm) */
typedef u32int      PTE;
/*e: type PTE(arm) */

/*s: struct Arch_Cpu(arm) */
//coupling: do not change the first field! Some assembly code assumes this order
struct Arch_Cpu {
    /*s: [[Cpu]] [[Arch]] mmu fields(arm) */
    PTE*    mmul1;      /* l1 for this processor */
    /*x: [[Cpu]] [[Arch]] mmu fields(arm) */
    // index in mmul1
    int mmul1lo;
    int mmul1hi;
    /*x: [[Cpu]] [[Arch]] mmu fields(arm) */
    int mmupid;
    /*e: [[Cpu]] [[Arch]] mmu fields(arm) */
    /*s: [[Cpu]] [[Arch]] float fields(arm) */
    /* vfp2 or vfp3 fpu */
    int havefp;
    int havefpvalid;
    int fpon;
    int fpconfiged;
    int fpnregs;
    ulong   fpscr;          /* sw copy */
    int fppid;          /* pid of last fault */
    uintptr fppc;           /* addr of last fault */
    int fpcnt;          /* how many consecutive at that addr */
    /*e: [[Cpu]] [[Arch]] float fields(arm) */
    /*s: [[Cpu]] [[Arch]] save of first five registers fields(arm) */
    /* save areas for exceptions, hold R0-R4 */
    u32int  sirq[5];
    u32int  sund[5];
    u32int  sabt[5];
    /*e: [[Cpu]] [[Arch]] save of first five registers fields(arm) */
    /*s: [[Cpu]] [[Arch]] other fields(arm) */
    uvlong  fastclock;      /* last sampled value */
    /*e: [[Cpu]] [[Arch]] other fields(arm) */
};
/*e: struct Arch_Cpu(arm) */
/*e: core/arm/dat_core.h */
