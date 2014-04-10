/*
 *  routines for things outside the PC model, like power management
 */
// this is actually only used in 386/ code.
struct PCArch
{
  char* id;
  int (*ident)(void);   /* this should be in the model */
  void  (*reset)(void);   /* this should be in the model */
  int (*serialpower)(int);  /* 1 == on, 0 == off */
  int (*modempower)(int); /* 1 == on, 0 == off */

  void  (*intrinit)(void);
  int (*intrenable)(Vctl*);
  int (*intrvecno)(int);
  int (*intrdisable)(int);
  void  (*introff)(void);
  void  (*intron)(void);

  void  (*clockenable)(void);
  uvlong  (*fastclock)(uvlong*);
  void  (*timerset)(uvlong);

  void  (*resetothers)(void); /* put other cpus into reset */
};
extern PCArch *arch;      /* PC architecture */


// this is used only in 386/ code
struct BIOS32ci {   /* BIOS32 Calling Interface */
  u32int  eax;
  u32int  ebx;
  u32int  ecx;
  u32int  edx;
  u32int  esi;
  u32int  edi;
};

//@Scheck: unnamed substructure
struct  FPstate     /* x87 fpu state */
{
  ushort  control;
  ushort  r1;
  // enum<fpsavestatus>
  ushort  status;
  ushort  r2;
  ushort  tag;
  ushort  r3;
  ulong pc;
  ushort  selector;
  ushort  r4;
  ulong operand;
  ushort  oselector;
  ushort  r5;
  uchar regs[80]; /* floating point registers */
};

//@Scheck: unnamed substructure
struct  FPssestate    /* SSE fp state */
{
  ushort  fcw;    /* control */
  ushort  fsw;    /* status */
  ushort  ftw;    /* tag */
  ushort  fop;    /* opcode */
  ulong fpuip;    /* pc */
  ushort  cs;   /* pc segment */
  ushort  r1;   /* reserved */
  ulong fpudp;    /* data pointer */
  ushort  ds;   /* data pointer segment */
  ushort  r2;
  ulong mxcsr;    /* MXCSR register state */
  ulong mxcsr_mask; /* MXCSR mask register */
  uchar xregs[480]; /* extended registers */
};

//@Scheck: unnamed substructure
struct  SFPssestate   /* SSE fp state with alignment slop */
{
  FPssestate;
  uchar alignpad[FPalign]; /* slop to allow copying to aligned addr */
  ulong magic;    /* debugging: check for overrun */
};

// used to be in devrtc.c, but to remove some backward deps had to be here
// it's really used only by nvram.c and devrtc.c
/*
 *  real time clock and non-volatile ram
 */

enum {
	Paddr=		0x70,	/* address port */
	PdataPort=		0x71,	/* data port */
};
extern Lock nvrtlock;
