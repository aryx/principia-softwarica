/*s: core/386/fns.h */
#include "../port/portfns_core.h"
#include "../port/portfns_concurrency.h"
#include "../port/portfns_memory.h"
#include "../port/portfns_files.h"
#include "../port/portfns_time.h"
#include "../port/portfns_interrupts.h"
#include "../port/portfns_processes.h"
#include "../port/portfns_misc.h"
#include "../port/portfns_console.h"
#include "../port/portfns_buses.h"
#include "../port/portfns_devices.h"
#include "../port/portfns_security.h"
#include "../port/portfns_network.h"
#include "../port/portfns_syscalls.h"
#include "../port/portfns_init.h"

/*s: fns.h declarations(x86) */

// <arch>/arch.c (called from main)
void   arch_cpuidprint(void);

// different signatures in different arch so cant factorize
void  arch_touser(void*);
int   (*arch_cmpswap)(long*, long, long);
void  (*arch_coherence)(void);
void  (*arch_cycles)(uvlong*);
/*s: function mmuflushtlb(x86) */
#define mmuflushtlb(mmupd) putcr3(mmupd)
/*e: function mmuflushtlb(x86) */

// fns_memory.h

// mmu.c (called from main.c)
void   mmuinit0(void); 
void*  tmpmap(Page*);
void   tmpunmap(void*);
// mmu.c (called from main.c/mp.c)
ulong* mmuwalk(ulong*, ulong, int, bool); 
// mmu.c (called from many arch specific)
void*  vmap(ulong, int);
void   vunmap(void*, int);
int    vmapsync(ulong);
// memory.c (called from main.c)
void   meminit(void); 
// memory.c (called from mmu.c)
kern_addr3 rampage(void);
void*  sigsearch(char*);
// memory.c (called from screen.c, pci.c)
ulong  upaalloc(int, int);
void   upareserve(ulong, int);
// l_misc.s (called from mmu.c)
//@Scheck: Assembly
void   invlpg(ulong);
// l.s (called from mmu.c)
//@Scheck: Assembly
void   ltr(ulong);
//@Scheck: Assembly
void   lidt(ushort[3]);
//@Scheck: Assembly
void   lgdt(ushort[3]);

// fns_init.h

// main.c (called from main.c/mp.c)
void   cpuinit(void); 
// l_misc.s (called from main.c)
//@Scheck: Assembly
void   halt(void);

// fns_processes.h

// trap.c (called from main.c)
void  trapenable(int, void (*)(Ureg*, void*), void*, char*);
void  trapinit0(void);
// trap.c (called from sdata.c, uarti...)
int   intrdisable(int, void (*)(Ureg *, void *), void*, int, char*);
// l_misc.s (called from mp.c, x86.c, trap.c)
void  rdmsr(int, vlong*);
// l_trap.s (called from trap.c)
//@Scheck: Assembly
void  vectortable(void);

// fns_devices.h

// kbd.c (called from main)
void  kbdinit(void);
void  kbdenable(void);
// cga.c (called from main)
void  cgapost(int);

// fns_arch.h

// devarch.c (called from main)
void  archinit(void);
// archgeneric.c (called from mp)
void  archrevert(void);
// l_misc.s 
//@Scheck: Assembly
void  cpuid(int, ulong regs[]);
// x86.c (called from main.c/mp.c)
int   cpuidentify(void);
//l_misc.c (called from i8253.c)
//@Scheck: Assembly
void  aamloop(int);
// l_misc.s (called from mp.c/archgeneric.c)
//@Scheck: Assembly
void  idle(void);
// i8253.c (called from x86.c)
void  guesscpuhz(int);
// archmp.c (called from mp.c)
void  syncclock(void);
// l_misc.c (called from arch specific)
//@Scheck: Assembly
void  wrmsr(int, vlong);

// fns_concurrency.h

// coherence possible values
//@Scheck: Assembly
void  mfence(void);
//@Scheck: Assembly
void  mb386(void);
//@Scheck: Assembly
void  mb586(void);
// l_misc.s (called from devarch)
// cmpswap possible value
int   cmpswap386(long*, long, long);
//@Scheck: Assembly
int   cmpswap486(long*, long, long);




// iomap.c
void (*hook_ioalloc)(void);

void  iofree(int);
void  ioinit(void);
int   ioalloc(int, int, int, char*);

//@Scheck: Assembly
int   bios32call(BIOS32ci*, u16int[3]);
int   bios32ci(BIOS32si*, BIOS32ci*);
BIOS32si* bios32open(char*);

void  dmaend(int); // called from port/devaudio.c!
long  dmasetup(int, void*, long, int);

int   dmainit(int, int);


//@Scheck: Assembly
void  fpclear(void);
//@Scheck: Assembly
void  fpinit(void);
//@Scheck: Assembly
void  fpoff(void);
//@Scheck: Assembly
void  fpon(void);


//@Scheck: Assembly
void  fpenv(Arch_FPsave*);
void  (*fprestore)(Arch_FPsave*);
void  (*fpsave)(Arch_FPsave*);
void  fpsavealloc(void);
void  fpsserestore(Arch_FPsave*);
//@Scheck: Assembly
void  fpsserestore0(Arch_FPsave*);
void  fpssesave(Arch_FPsave*);
//@Scheck: Assembly
void  fpssesave0(Arch_FPsave*);
//ulong fpstatus(void);
//@Scheck: Assembly
void  fpx87restore(Arch_FPsave*);
//@Scheck: Assembly
void  fpx87save(Arch_FPsave*);

//@Scheck: Assembly
ulong getcr0(void);
//@Scheck: Assembly
ulong getcr2(void);
//@Scheck: Assembly
ulong getcr3(void);
//@Scheck: Assembly
ulong getcr4(void);

//@Scheck: Assembly
void  putcr0(ulong);
//@Scheck: Assembly
void  putcr3(ulong);
//@Scheck: Assembly
void  putcr4(ulong);


int   i8042auxcmd(int);
void  i8042auxenable(void (*)(int, int));
void  i8042reset(void);

void  i8250console(void);
void* i8250alloc(int, int, int);

void  i8253enable(void);
void  i8253init(void);
uvlong  i8253read(uvlong*);
void  i8253timerset(uvlong);

int   i8259disable(int);
int   i8259enable(Vctl*);
void  i8259init(void);
int   i8259isr(int);
void  i8259on(void);
void  i8259off(void);
int   i8259vecno(int);

//@Scheck: Assembly
int   inb(int);
void  insb(int, void*, int); // used only by ether8390 for now
//@Scheck: Assembly
ushort ins(int);
//@Scheck: Assembly
void  inss(int, void*, int);
//@Scheck: Assembly
ulong inl(int);

//@Scheck: Assembly
void  outb(int, int);
void  outsb(int, void*, int); // used only by ether8390 for now
//@Scheck: Assembly
void  outs(int, ushort);
//@Scheck: Assembly
void  outss(int, void*, int);
//@Scheck: Assembly
void  outl(int, ulong);

//int mtrr(uvlong, uvlong, char *);
//void mtrrclock(void);
//int mtrrprint(char *, long);

// nvram.c (called from devfloppy.c and memory.c)
uchar nvramread(int);
void  nvramwrite(int, uchar);

int   pcicfgr8(Pcidev*, int);
int   pcicfgr16(Pcidev*, int);
int   pcicfgr32(Pcidev*, int);
void  pcicfgw8(Pcidev*, int, int);
void  pcicfgw32(Pcidev*, int, int);
void  pciclrbme(Pcidev*);
Pcidev* pcimatch(Pcidev*, int, int);
Pcidev* pcimatchtbdf(int);
void  pcireset(void);
void  pcisetbme(Pcidev*);
//void  pcicfgw16(Pcidev*, int, int);
//void  pciclrioe(Pcidev*);
//void  pciclrmwi(Pcidev*);
//int   pcigetpms(Pcidev*);
//int   pciscan(int, Pcidev**);
//void  pcisetioe(Pcidev*);
//void  pcisetmwi(Pcidev*);
//int   pcisetpms(Pcidev*, int);

int pcmspecial(char*, ISAConf*);
//int (*_pcmspecial)(char *, ISAConf *);
//void  pcmcisread(PCMslot*);
//int   pcmcistuple(int, int, int, void*, int);
//void  pcmspecialclose(int);
//void  (*_pcmspecialclose)(int);
//PCMmap* pcmmap(int, ulong, int, int);
//void  pcmunmap(int, PCMmap*);


/*s: fns.h macros(x86) */
/*s: function KADDR(x86) */
#define KADDR(pa)  arch_kaddr(pa)
/*e: function KADDR(x86) */
/*s: function PADDR(x86) */
#define PADDR(ka)  arch_paddr((kern_addr3)(ka))
/*e: function PADDR(x86) */

#define BIOSSEG(a)  KADDR(((uint)(a))<<4)

#define L16GET(p) (((p)[1]<<8)|(p)[0])
#define L32GET(p) (((u32int)L16GET((p)+2)<<16)|L16GET(p))
/*e: fns.h macros(x86) */
/*e: fns.h declarations(x86) */
/*e: core/386/fns.h */
