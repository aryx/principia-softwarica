/*s: dat_arch.h */

//*****************************************************************************
// IO Map
//*****************************************************************************

// All the ref<IOMap> here are references to IOMap in the array<IOMap> of 
// Iomapalloc.maps (pool allocator)

/*s: struct IOMap */
struct IOMap
{
    // TODO: why this has to be the first field of IOMap?! otherwise get fault??
    // list<ref<IOMap>> of Iomapalloc.free
    IOMap   *next;

    char    tag[13];
    ulong   start;
    ulong   end;

    // extra
    bool    reserved;
};
/*e: struct IOMap */

/*s: struct Iomapalloc */
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
/*e: struct Iomapalloc */

// array<IMap> alloced statically in maps
extern struct Iomapalloc iomap;

//*****************************************************************************
// PC Architecture hooks (interrupts, clocks, power, reset)
//*****************************************************************************

/*
 *  routines for things outside the PC model, like power management
 */
/*s: struct PCArch */
struct PCArch
{
  char* id;

  int (*ident)(void);   /* this should be in the model */
  void  (*reset)(void);   /* this should be in the model */

  int (*serialpower)(int);  /* 1 == on, 0 == off */
  int (*modempower)(int); /* 1 == on, 0 == off */

  // interrupts
  void  (*intrinit)(void);
  int (*intrenable)(Vctl*);
  int (*intrvecno)(int);
  int (*intrdisable)(int);
  void  (*introff)(void);
  void  (*intron)(void);

  // clock, timer
  void  (*clockenable)(void);
  uvlong  (*fastclock)(uvlong*);
  void  (*timerset)(uvlong);

  void  (*resetothers)(void); /* put other cpus into reset */
};
/*e: struct PCArch */
extern PCArch *arch;      /* PC architecture */
extern PCArch archgeneric;

//*****************************************************************************
// Co processor
//*****************************************************************************

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

/*s: struct X86type */
struct X86type {
    int family;
    int model;
    int aalcycles;
    char*   name;
};
/*e: struct X86type */

extern X86type *cputype;


/*
 *  hardware info about a device
 */
/*s: struct Devport */
struct Devport {
  ulong port; 
  int size;
};
/*e: struct Devport */

/*s: struct DevConf */
struct DevConf
{
  ulong intnum;     /* interrupt number */
  char  *type;      /* card type, malloced */
  int nports;     /* Number of ports */
  Devport *ports;     /* The ports themselves */
};
/*e: struct DevConf */
/*e: dat_arch.h */
