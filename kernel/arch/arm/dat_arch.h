/*s: arch/arm/dat_arch.h */

/*s: enum _anon_ (arch/arm/dat_arch.h)(arm) */
enum {
    Maxfpregs   = 32,   /* could be 16 or 32, see Mach.fpnregs */
    Nfpctlregs  = 16,
};
/*e: enum _anon_ (arch/arm/dat_arch.h)(arm) */
/*s: struct Soc(arm) */
struct Soc {            /* SoC dependent configuration */
    ulong   dramsize;

    uintptr physio;
    uintptr busdram;
    uintptr busio;
    /*s: [[Soc]] mmu fields */
    u32int  l1ptedramattrs;
    u32int  l2ptedramattrs;
    /*e: [[Soc]] mmu fields */
    /*s: [[Soc]] other fields */
    uintptr armlocal;
    /*e: [[Soc]] other fields */
};
/*e: struct Soc(arm) */
extern Soc soc;
/*e: arch/arm/dat_arch.h */
