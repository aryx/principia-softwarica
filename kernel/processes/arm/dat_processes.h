/*s: processes/arm/dat_processes.h */

/*s: constant AOUT_MAGIC(arm) */
/*
 *  parameters for sysproc.c
 */
// E_MAGIC is defined in include/a.out.h
#define AOUT_MAGIC  (E_MAGIC)
/*e: constant AOUT_MAGIC(arm) */

/*s: enum _anon_ (arch/arm/dat_processes.h)(arm) */
enum {
    Maxfpregs   = 32,   /* could be 16 or 32, see Mach.fpnregs */
    Nfpctlregs  = 16,
};
/*e: enum _anon_ (arch/arm/dat_processes.h)(arm) */

/*s: struct Arch_FPsave(arm) */
/*
 * emulated or vfp3 floating point
 */
struct Arch_FPsave
{
    /*
     * vfp3 with ieee fp regs; uvlong is sufficient for hardware but
     * each must be able to hold an Internal from fpi.h for sw emulation.
     */
    ulong   regs[Maxfpregs][3];
    /*s: [[Proc]] [[Arch]] [[FPsave]] other fields */
    int fpstate; // vs Proc.fpstate??

    ulong   status;
    ulong   control;

    uintptr pc;     /* of failed fp instr. */
    /*e: [[Proc]] [[Arch]] [[FPsave]] other fields */
};
/*e: struct Arch_FPsave(arm) */

/*e: processes/arm/dat_processes.h */
