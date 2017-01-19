
/*
 *  parameters for sysproc.c
 */
#define AOUT_MAGIC	(E_MAGIC)

/*
 * emulated or vfp3 floating point
 */
struct ArchFPsave
{
	ulong	status;
	ulong	control;
	/*
	 * vfp3 with ieee fp regs; uvlong is sufficient for hardware but
	 * each must be able to hold an Internal from fpi.h for sw emulation.
	 */
	ulong	regs[Maxfpregs][3];

	int	fpstate;

	uintptr	pc;		/* of failed fp instr. */
};

