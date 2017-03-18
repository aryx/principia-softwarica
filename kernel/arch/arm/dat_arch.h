/*s: arch/arm/dat_arch.h */

/*s: struct Soc(arm) */
struct Soc {            /* SoC dependent configuration */
    ulong   dramsize;
    uintptr physio;
    /*s: [[Soc]] bus fields */
    uintptr busdram;
    /*x: [[Soc]] bus fields */
    uintptr busio;
    /*e: [[Soc]] bus fields */
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
