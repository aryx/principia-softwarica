/*s: dat_arch.h */

//*****************************************************************************
// IO Map
//*****************************************************************************

// All the ref<IOMap> here are references to IOMap in the array<IOMap> of 
// Iomapalloc.maps (pool allocator)

/*s: struct IOMap(x86) */
struct IOMap
{
    char    tag[13];
    ulong   start;
    ulong   end;

    // extra
    // list<ref<IOMap>> of Iomapalloc.free
    IOMap   *next;

    bool    reserved;
};
/*e: struct IOMap(x86) */

/*s: struct Iomapalloc(x86) */
struct Iomapalloc
{
    // ??
    IOMap   *m;

    // list<ref<IOMap>> (next = IOMap.next)
    IOMap   *free;
    // array<IOMAP> pool
    IOMap   maps[32];   /* some initial free maps */

    // extra
    Lock;
    QLock   ql;     /* lock for reading map */
};
/*e: struct Iomapalloc(x86) */

// array<IMap> alloced statically in maps
extern struct Iomapalloc iomap;

//*****************************************************************************
// PC Architecture hooks (interrupts, clocks, power, reset)
//*****************************************************************************

/*
 *  routines for things outside the PC model, like power management
 */
/*s: struct PCArch(x86) */
struct PCArch
{
  char* id;

  int (*ident)(void);   /* this should be in the model */
  void  (*reset)(void);   /* this should be in the model */
  void  (*resetothers)(void); /* put other cpus into reset */

  // interrupts
  /*s: [[PCArch]] interrupt methods fields(x86) */
  void  (*intrinit)(void);
  int (*intrenable)(Vctl*);
  int (*intrvecno)(int);
  int (*intrdisable)(int);
  void  (*introff)(void);
  void  (*intron)(void);
  /*e: [[PCArch]] interrupt methods fields(x86) */
  // clock, timer
  /*s: [[PCArch]] time methods fields(x86) */
  void  (*clockenable)(void);
  uvlong  (*fastclock)(uvlong*);
  void  (*timerset)(uvlong);
  /*e: [[PCArch]] time methods fields(x86) */
  // power
  /*s: [[PCArch]] power methods fields(x86) */
  int (*serialpower)(int);  /* 1 == on, 0 == off */
  int (*modempower)(int); /* 1 == on, 0 == off */
  /*e: [[PCArch]] power methods fields(x86) */
};
/*e: struct PCArch(x86) */
extern PCArch *arch;      /* PC architecture */
extern PCArch archgeneric;

//*****************************************************************************
// Co processor
//*****************************************************************************

/*s: struct FPstate(x86) */
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
/*e: struct FPstate(x86) */

/*s: struct FPssestate(x86) */
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
/*e: struct FPssestate(x86) */

/*s: struct  SFPssestate(x86) */
struct  SFPssestate   /* SSE fp state with alignment slop */
{
  FPssestate;
  uchar alignpad[FPalign]; /* slop to allow copying to aligned addr */
  ulong magic;    /* debugging: check for overrun */
};
/*e: struct  SFPssestate(x86) */

//*****************************************************************************
// Misc
//*****************************************************************************

struct BIOS32ci {   /* BIOS32 Calling Interface */
  u32int  eax;
  u32int  ebx;
  u32int  ecx;
  u32int  edx;
  u32int  esi;
  u32int  edi;
};


// used to be in devrtc.c, but to remove some backward deps had to be here
// it's really used only by nvram.c and devrtc.c
/*
 *  real time clock and non-volatile ram
 */

enum {
    Paddr=      0x70,   /* address port */
    PdataPort=      0x71,   /* data port */
};
extern Lock nvrtlock;

/*s: struct X86type(x86) */
struct X86type {
    int family;
    int model;
    int aalcycles;
    char*   name;
};
/*e: struct X86type(x86) */

extern X86type *cputype;


/*
 *  hardware info about a device
 */
/*s: struct Devport(x86) */
struct Devport {
    ulong port; 
    int size;
};
/*e: struct Devport(x86) */

/*s: struct DevConf(x86) */
struct DevConf
{
    ulong intnum;     /* interrupt number */
    char  *type;      /* card type, malloced */
    int nports;     /* Number of ports */
    Devport *ports;     /* The ports themselves */
};
/*e: struct DevConf(x86) */
/*e: dat_arch.h */
