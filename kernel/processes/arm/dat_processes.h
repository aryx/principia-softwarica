/*s: processes/arm/dat_processes.h */

/*s: constant AOUT_MAGIC(arm) */
/*
 *  parameters for sysproc.c
 */
#define AOUT_MAGIC  (E_MAGIC)
/*e: constant AOUT_MAGIC(arm) */

/*s: struct Arch_FPsave(arm) */
/*
 * emulated or vfp3 floating point
 */
struct Arch_FPsave
{
    ulong   status;
    ulong   control;
    /*
     * vfp3 with ieee fp regs; uvlong is sufficient for hardware but
     * each must be able to hold an Internal from fpi.h for sw emulation.
     */
    ulong   regs[Maxfpregs][3];

    int fpstate;

    uintptr pc;     /* of failed fp instr. */
};
/*e: struct Arch_FPsave(arm) */

/*e: processes/arm/dat_processes.h */
