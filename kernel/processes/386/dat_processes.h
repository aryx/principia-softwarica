/*s: processes/386/dat_processes.h */

/*s: constant [[AOUT_MAGIC]](x86) */
/*
 *  parameters for sysproc.c
 */
// I_MAGIC is defined in include/a.out.h, I for INTEL?
#define AOUT_MAGIC  (I_MAGIC)
/*e: constant [[AOUT_MAGIC]](x86) */

//*****************************************************************************
// Proc extensions
//*****************************************************************************

/*s: union ArchFPSave(x86) */
/*
 * the FP regs must be stored here, not somewhere pointed to from here.
 * port code assumes this.
 */
union Arch_FPsave {
    FPstate;
    SFPssestate;
};
/*e: union ArchFPSave(x86) */

/*e: processes/386/dat_processes.h */
