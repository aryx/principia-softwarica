
//*****************************************************************************
// Proc extensions
//*****************************************************************************

/*
 * the FP regs must be stored here, not somewhere pointed to from here.
 * port code assumes this.
 */
// could be renamed ArchProcFPSave (used both in Proc and Mach)
union ArchFPsave {
  FPstate;
  SFPssestate;
};

/*
 *  things saved in the Proc structure during a notify
 */
//@Scheck: unnamed substructure
struct ArchProcNotsave
{
  ulong svflags;
  ulong svcs;
  ulong svss;
};

//*****************************************************************************
// Interrupts
//*****************************************************************************
// used only in 386/, so could be put in arch/ but more specific to the .c here
// used to be in io.h but more important than just a set of enums for IO
// so put here.

enum {
  VectorNMI = 2,    /* non-maskable interrupt */
  VectorBPT = 3,    /* breakpoint */
  VectorUD  = 6,    /* invalid opcode exception */
  VectorCNA = 7,    /* coprocessor not available */
  Vector2F  = 8,    /* double fault */
  VectorCSO = 9,    /* coprocessor segment overrun */
  VectorPF  = 14,   /* page fault */
  Vector15  = 15,   /* reserved */
  VectorCERR  = 16,   /* coprocessor error */

  VectorPIC = 32,   /* external i8259 interrupts */

  IrqCLOCK  = 0,
  IrqKBD    = 1,
  IrqUART1  = 3,
  IrqUART0  = 4,
  IrqPCMCIA = 5,
  IrqFLOPPY = 6,
  IrqLPT    = 7,
  IrqIRQ7   = 7,
  IrqAUX    = 12,   /* PS/2 port */
  IrqIRQ13  = 13,   /* coprocessor on 386 */
  IrqATA0   = 14,
  IrqATA1   = 15,
  MaxIrqPIC = 15,

  VectorLAPIC = VectorPIC+16, /* local APIC interrupts */
  IrqLINT0  = 16,   /* LINT[01] must be offsets 0 and 1 */
  IrqLINT1  = 17,
  IrqTIMER  = 18,
  IrqERROR  = 19,
  IrqPCINT  = 20,
  IrqSPURIOUS = 31,   /* must have bits [3-0] == 0x0F */
  MaxIrqLAPIC = 31,

  //!!! int 64 = way to jump in plan9 OS !!!
  VectorSYSCALL = 64,

  VectorAPIC  = 65,   /* external APIC interrupts */
  MaxVectorAPIC = 255,
};

struct Vctl {

  bool isintr;     /* interrupt or fault/trap */
  int irq;

  void  (*f)(Ureg*, void*); /* handler to call */
  void* a;      /* argument to call it with */

  char  name[KNAMELEN];   /* of driver */
  int tbdf; //?

  // interrupt service routine
  int (*isr)(int);    /* get isr bit for this irq */
  int (*eoi)(int);    /* eoi */

  // extra

  // list<Vctl> of vctl[vno], xalloc'ed (should not have that many so ok)
  Vctl* next;     /* handlers on this vector */
};

// array<list<Vctl>>
//IMPORTANT: static Vctl *vctl[256]; (in trap.c)

//*****************************************************************************
// Timer
//*****************************************************************************

// used only in 386/, so could be put in arch/ but more specific to the .c here
// used only in i8253.c but important so put here

struct I8253
{
	ulong	period;		/* current clock period */
	bool	enabled;
	uvlong	hz;

	ushort	last;		/* last value of clock 1 */
	uvlong	ticks;		/* cumulative ticks of counter 1 */

	ulong	periodset;

  // extra
	Lock;
};
//IMPORTANT: I8253 i8253; (in i8253.c)

//IMPORTANT: also is interrupt i8253clock() calling clock.c:timerintr()
