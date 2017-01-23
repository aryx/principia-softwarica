/*s: fns.h */
#include "../port/portfns_core.h"
#include "../port/portfns_concurrency.h"
#include "../port/portfns_memory.h"
#include "../port/portfns_files.h"
#include "../port/portfns_processes.h"
#include "../port/portfns_misc.h"
#include "../port/portfns_console.h"
#include "../port/portfns_buses.h"
#include "../port/portfns_devices.h"
#include "../port/portfns_security.h"
#include "../port/portfns_network.h"
#include "../port/portfns_init.h"

/*s: fns.h declarations(x86) */

// could be in portfns.h, same type in every arch, but called from arch-specific

// used by $CONF.c and main.c
void    bootlinks(void);
void  links(void);
//@Scheck: Assembly
void    forkret(void);

// used by main.c
void    cpuinit(void);
// used by main.c
void mmuinit0(void);
void mmuinit(void);
ulong*  mmuwalk(ulong*, ulong, int, bool);

void meminit(void);
void memorysummary(void);

//@Scheck: Assembly
void  touser(void*);
void  trapenable(int, void (*)(Ureg*, void*), void*, char*);
void  trapinit(void);
void  trapinit0(void);
void  intrenable(int, void (*)(Ureg*, void*), void*, int, char*);
int intrdisable(int, void (*)(Ureg *, void *), void*, int, char*);
//void  introff(void);
//void  intron(void);

void  screeninit(void);
void  (*screenputs)(char*, int);

void  kbdenable(void);
void  kbdinit(void);

void  procrestore(Proc*);
void  procsave(Proc*);
void  procsetup(Proc*);

Dirtab* addarchfile(char*, int, long(*)(Chan*,void*,long,vlong), long(*)(Chan*,void*,long,vlong));
void  archinit(void);
void  archrevert(void);

int (*cmpswap)(long*, long, long);
//@Scheck: Assembly
int cmpswap486(long*, long, long);

//@Scheck: Assembly
void  cpuid(int, ulong regs[]);
int cpuidentify(void);
void  cpuidprint(void);
void  (*cycles)(uvlong*);

//int (*isaconfig)(char*, int, ISAConf*);

//#define evenaddr(x)       /* x86 doesn't care */
void	validalign(uintptr, unsigned);

//@Scheck: Assembly
void  idle(void);

void  syscallfmt(int syscallno, ulong pc, va_list list);
void  sysretfmt(int syscallno, va_list list, long ret, uvlong start, uvlong stop);

void  guesscpuhz(int);

//@Scheck: Assembly
void  halt(void);




void  cgapost(int);
//void  clockintr(Ureg*, void*);

// iomap.c
void (*hook_ioalloc)(void);

void  iofree(int);
void  ioinit(void);
int ioalloc(int, int, int, char*);
//int ioreserve(int, int, int, char*);


//@Scheck: Assembly
void  aamloop(int);
//void  acpiscan(void (*func)(uchar *));

//@Scheck: Assembly
int bios32call(BIOS32ci*, u16int[3]);
int bios32ci(BIOS32si*, BIOS32ci*);
BIOS32si* bios32open(char*);
//void  bios32close(BIOS32si*);

int dmainit(int, int);
void  dmaend(int);
long  dmasetup(int, void*, long, int);
//int dmacount(int);
//int dmadone(int);


//@Scheck: Assembly
void  fpclear(void);
//@Scheck: Assembly
void  fpenv(Arch_FPsave*);
//@Scheck: Assembly
void  fpinit(void);
//@Scheck: Assembly
void  fpoff(void);
//@Scheck: Assembly
void  fpon(void);
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


int i8042auxcmd(int);
//int i8042auxcmds(uchar*, int);
void  i8042auxenable(void (*)(int, int));
void  i8042reset(void);
void  i8250console(void);
void* i8250alloc(int, int, int);
void  i8253enable(void);
void  i8253init(void);
uvlong  i8253read(uvlong*);
void  i8253timerset(uvlong);
int i8259disable(int);
int i8259enable(Vctl*);
void  i8259init(void);
int i8259isr(int);
void  i8259on(void);
void  i8259off(void);
int i8259vecno(int);

//@Scheck: Assembly
int inb(int);
void  insb(int, void*, int); // used only by ether8390 for now
//@Scheck: Assembly
ushort  ins(int);
//@Scheck: Assembly
void  inss(int, void*, int);
//@Scheck: Assembly
ulong inl(int);
//void  insl(int, void*, int);

//@Scheck: Assembly
void  outb(int, int);
void  outsb(int, void*, int); // used only by ether8390 for now
//@Scheck: Assembly
void  outs(int, ushort);
//@Scheck: Assembly
void  outss(int, void*, int);
//@Scheck: Assembly
void  outl(int, ulong);
//void  outsl(int, void*, int);



//@Scheck: Assembly
void  invlpg(ulong);


/*s: function kmapinval(x86) */
#define kmapinval()
/*e: function kmapinval(x86) */

//@Scheck: Assembly
void  lgdt(ushort[3]);
//@Scheck: Assembly
void  lidt(ushort[3]);
void  ltr(ulong);
//@Scheck: Assembly
void  mb386(void);
//@Scheck: Assembly
void  mb586(void);

//@Scheck: Assembly
void  mfence(void);

/*s: function mmuflushtlb(x86) */
#define mmuflushtlb(mmupd) putcr3(mmupd)
/*e: function mmuflushtlb(x86) */

//int mtrr(uvlong, uvlong, char *);
//void  mtrrclock(void);
//int mtrrprint(char *, long);

uchar nvramread(int);
void  nvramwrite(int, uchar);

int pcicfgr8(Pcidev*, int);
int pcicfgr16(Pcidev*, int);
int pcicfgr32(Pcidev*, int);
void  pcicfgw8(Pcidev*, int, int);
//void  pcicfgw16(Pcidev*, int, int);
void  pcicfgw32(Pcidev*, int, int);
void  pciclrbme(Pcidev*);
//void  pciclrioe(Pcidev*);
//void  pciclrmwi(Pcidev*);
//int pcigetpms(Pcidev*);
Pcidev* pcimatch(Pcidev*, int, int);
Pcidev* pcimatchtbdf(int);
void  pcireset(void);
//int pciscan(int, Pcidev**);
void  pcisetbme(Pcidev*);
//void  pcisetioe(Pcidev*);
//void  pcisetmwi(Pcidev*);
//int pcisetpms(Pcidev*, int);

int pcmspecial(char*, ISAConf*);
//int (*_pcmspecial)(char *, ISAConf *);
//void  pcmcisread(PCMslot*);
//int pcmcistuple(int, int, int, void*, int);
//void  pcmspecialclose(int);
//void  (*_pcmspecialclose)(int);
//PCMmap* pcmmap(int, ulong, int, int);
//void  pcmunmap(int, PCMmap*);

kern_addr3 rampage(void);
//@Scheck: Assembly

void  rdmsr(int, vlong*);

//void  realmode(Ureg*);

void* sigsearch(char*);

void  syncclock(void);

//int pdmap(ulong*, ulong, ulong, int);

void* tmpmap(Page*);
void  tmpunmap(void*);

void* vmap(ulong, int);
int vmapsync(ulong);
void  vunmap(void*, int);

ulong upaalloc(int, int);
void  upareserve(ulong, int);
//void  upafree(ulong, int);

//@Scheck: Assembly
void  vectortable(void);

//@Scheck: Assembly
void  wbinvd(void);
//@Scheck: Assembly
void  wrmsr(int, vlong);
//int xchgw(ushort*, int);

//int iounused(int start, int end); not used anymore in vga.c

/*s: fns.h macros(x86) */
/*s: function KADDR(x86) */
#define KADDR(pa)  arch_kaddr(pa)
/*e: function KADDR(x86) */
/*s: function PADDR(x86) */
#define PADDR(ka)  arch_paddr((kern_addr3)(ka))
/*e: function PADDR(x86) */

// used in devaudio, maybe could remove it
#define dcflush(a, b)

#define BIOSSEG(a)  KADDR(((uint)(a))<<4)

#define L16GET(p) (((p)[1]<<8)|(p)[0])
#define L32GET(p) (((u32int)L16GET((p)+2)<<16)|L16GET(p))
/*e: fns.h macros(x86) */
/*e: fns.h declarations(x86) */
/*e: fns.h */
