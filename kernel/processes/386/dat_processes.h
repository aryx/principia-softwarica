/*s: dat_processes.h */

/*s: constant AOUT_MAGIC(x86) */
/*
 *  parameters for sysproc.c
 */
// I_MAGIC is defined in include/a.out.h, I for INTEL?
#define AOUT_MAGIC  (I_MAGIC)
/*e: constant AOUT_MAGIC(x86) */

//*****************************************************************************
// Proc extensions
//*****************************************************************************

/*s: union ArchFPSave(x86) */
/*
 * the FP regs must be stored here, not somewhere pointed to from here.
 * port code assumes this.
 */
// could be renamed ArchProcFPSave (used both in Proc and Cpu though)
union ArchFPsave {
    FPstate;
    SFPssestate;
};
/*e: union ArchFPSave(x86) */

//*****************************************************************************
// Interrupts
//*****************************************************************************
// Used only in 386/, so could be put in arch/, but used by the .c here.
// Used to be in io.h but more important than just a set of enums for IO
// so put here.

/*s: enum vector(x86) */
enum {
    /*s: enum vector cases(x86) */
        VectorNMI = 2,    /* non-maskable interrupt */
        VectorBPT = 3,    /* breakpoint */
        VectorUD  = 6,    /* invalid opcode exception */
        VectorCNA = 7,    /* coprocessor not available */
        Vector2F  = 8,    /* double fault */
        VectorCSO = 9,    /* coprocessor segment overrun */
        VectorPF  = 14,   /* page fault */ //!!! page fault interrupt
        Vector15  = 15,   /* reserved */
        VectorCERR  = 16,   /* coprocessor error */
    /*x: enum vector cases(x86) */
        VectorPIC = 32,   /* external i8259 interrupts */
    /*x: enum vector cases(x86) */
        //!!! int 64, or int 0x40 = way to jump in plan9 OS !!!
        // VectorSYSCALL = 64, in mem.h because used by Assembly too
    /*x: enum vector cases(x86) */
        VectorLAPIC = VectorPIC+16, /* local APIC interrupts */
        VectorAPIC  = 65,   /* external APIC interrupts */
        MaxVectorAPIC = 255,
    /*e: enum vector cases(x86) */
};
/*e: enum vector(x86) */

/*s: enum irq(x86) */
enum {
    IrqCLOCK  = 0, // !!! clock interrupt
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
  
    IrqLINT0  = 16,   /* LINT[01] must be offsets 0 and 1 */
    IrqLINT1  = 17,
    IrqTIMER  = 18, // for alarms
    IrqERROR  = 19,
    IrqPCINT  = 20,
    IrqSPURIOUS = 31,   /* must have bits [3-0] == 0x0F */

    MaxIrqLAPIC = 31,
};
/*e: enum irq(x86) */
  

/*s: struct Vctl(x86) */
struct Vctl {

    bool isintr;     /* interrupt or fault/trap */
    int irq; // if interrupt
  
    void  (*f)(Ureg*, void*); /* handler to call */
    void* a;      /* argument to call it with */
  
    char  name[KNAMELEN];   /* of driver */
    int tbdf; // /* type+bus+device+function */ ??
  
    /*s: [[Vctl]] other fields(x86) */
    // interrupt service routine
    int (*isr)(int);    /* get isr bit for this irq */
    int (*eoi)(int);    /* eoi */
    /*e: [[Vctl]] other fields(x86) */
    // extra
    /*s: [[Vctl]] extra fields(x86) */
    // list<ref_own<Vctl> of vctl[vno], xalloc'ed (should not have that many so ok)
    Vctl* next;     /* handlers on this vector */
    /*e: [[Vctl]] extra fields(x86) */
};
/*e: struct Vctl(x86) */

// array<list<Vctl>>, xalloc'ed
//IMPORTANT: static Vctl *vctl[256]; (in trap.c)

//*****************************************************************************
// Timer
//*****************************************************************************

// Used only in 386/, so could be put in arch/ but used by the .c here.
// Actually used only in i8253.c but important so put here.

/*s: struct I8253(x86) */
struct I8253
{
    ulong   period;     /* current clock period */
    bool    enabled;

    uvlong  hz;

    ushort  last;       /* last value of clock 1 */
    uvlong  ticks;      /* cumulative ticks of counter 1 */

    ulong   periodset;

    // extra
    Lock;
};
/*e: struct I8253(x86) */

//IMPORTANT: I8253 i8253; (in i8253.c)
//IMPORTANT: also is interrupt i8253clock() calling clock.c:timerintr()
/*e: dat_processes.h */
