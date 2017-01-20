
enum {
	Maxfpregs	= 32,	/* could be 16 or 32, see Mach.fpnregs */
	Nfpctlregs	= 16,
};

typedef struct Soc Soc;

struct Soc {			/* SoC dependent configuration */
	ulong	dramsize;
	uintptr	physio;
	uintptr	busdram;
	uintptr	busio;
	uintptr	armlocal;
	u32int	l1ptedramattrs;
	u32int	l2ptedramattrs;
};
extern Soc soc;
