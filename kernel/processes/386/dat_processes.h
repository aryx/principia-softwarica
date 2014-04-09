
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

/*
 * the FP regs must be stored here, not somewhere pointed to from here.
 * port code assumes this.
 */
union FPsave {
  FPstate;
  SFPssestate;
};



/*
 *  things saved in the Proc structure during a notify
 */
//@Scheck: unnamed substructure
struct ArchNotsave
{
  ulong svflags;
  ulong svcs;
  ulong svss;
};


