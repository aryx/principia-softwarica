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

/*s: fns.h declarations */

// used by $CONF.c and main.c
void    bootlinks(void);

// defined in lib/latin1.c, used only in 386
long    latin1(Rune*, int);

// used by main.c
void    cpuinit(void);

// used by main.c
void mmuinit0(void);

// iomap.c
void (*hook_ioalloc)();

//@Scheck: Assembly
void    forkret(void);
//@Scheck: Assembly
void  aamloop(int);
//void  acpiscan(void (*func)(uchar *));
Dirtab* addarchfile(char*, int, long(*)(Chan*,void*,long,vlong), long(*)(Chan*,void*,long,vlong));
void  archinit(void);
void  archrevert(void);
//@Scheck: Assembly
int bios32call(BIOS32ci*, u16int[3]);
int bios32ci(BIOS32si*, BIOS32ci*);
//void  bios32close(BIOS32si*);
BIOS32si* bios32open(char*);
void  cgapost(int);
//void  clockintr(Ureg*, void*);
int (*cmpswap)(long*, long, long);
//@Scheck: Assembly
int cmpswap486(long*, long, long);
//@Scheck: Assembly
void  cpuid(int, ulong regs[]);
int cpuidentify(void);
void  cpuidprint(void);
void  (*cycles)(uvlong*);
//int dmacount(int);
//int dmadone(int);
void  dmaend(int);
int dmainit(int, int);
long  dmasetup(int, void*, long, int);
#define evenaddr(x)       /* x86 doesn't care */
//@Scheck: Assembly
void  fpclear(void);
//@Scheck: Assembly
void  fpenv(ArchFPsave*);
//@Scheck: Assembly
void  fpinit(void);
//@Scheck: Assembly
void  fpoff(void);
//@Scheck: Assembly
void  fpon(void);
void  (*fprestore)(ArchFPsave*);
void  (*fpsave)(ArchFPsave*);
void  fpsavealloc(void);
void  fpsserestore(ArchFPsave*);
//@Scheck: Assembly
void  fpsserestore0(ArchFPsave*);
void  fpssesave(ArchFPsave*);
//@Scheck: Assembly
void  fpssesave0(ArchFPsave*);
//ulong fpstatus(void);
//@Scheck: Assembly
void  fpx87restore(ArchFPsave*);
//@Scheck: Assembly
void  fpx87save(ArchFPsave*);
//@Scheck: Assembly
ulong getcr0(void);
//@Scheck: Assembly
ulong getcr2(void);
//@Scheck: Assembly
ulong getcr3(void);
//@Scheck: Assembly
ulong getcr4(void);
void  guesscpuhz(int);
//@Scheck: Assembly
void  halt(void);
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
void  idle(void);
void  idlehands(void);
//@Scheck: Assembly
int inb(int);
//void  insb(int, void*, int);
//@Scheck: Assembly
ushort  ins(int);
//@Scheck: Assembly
void  inss(int, void*, int);
//@Scheck: Assembly
ulong inl(int);
//void  insl(int, void*, int);
int intrdisable(int, void (*)(Ureg *, void *), void*, int, char*);
void  intrenable(int, void (*)(Ureg*, void*), void*, int, char*);
//void  introff(void);
//void  intron(void);
//@Scheck: Assembly
void  invlpg(ulong);
void  iofree(int);
void  ioinit(void);
int ioalloc(int, int, int, char*);
//int ioreserve(int, int, int, char*);
int (*isaconfig)(char*, int, ISAConf*);
void  kbdenable(void);
void  kbdinit(void);
/*s: function kmapinval */
#define kmapinval()
/*e: function kmapinval */
//@Scheck: Assembly
void  lgdt(ushort[3]);
//@Scheck: Assembly
void  lidt(ushort[3]);
void  links(void);
//@Scheck: Assembly
void  ltr(ulong);
//@Scheck: Assembly
void  mb386(void);
//@Scheck: Assembly
void  mb586(void);
void  meminit(void);
void  memorysummary(void);
//@Scheck: Assembly
void  mfence(void);
/*s: function mmuflushtlb */
#define mmuflushtlb(mmupd) putcr3(mmupd)
/*e: function mmuflushtlb */
void  mmuinit(void);
ulong*  mmuwalk(ulong*, ulong, int, bool);
//int mtrr(uvlong, uvlong, char *);
//void  mtrrclock(void);
//int mtrrprint(char *, long);
uchar nvramread(int);
void  nvramwrite(int, uchar);
//@Scheck: Assembly
void  outb(int, int);
//void  outsb(int, void*, int);
//@Scheck: Assembly
void  outs(int, ushort);
//@Scheck: Assembly
void  outss(int, void*, int);
//@Scheck: Assembly
void  outl(int, ulong);
//void  outsl(int, void*, int);
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
//void  pcmcisread(PCMslot*);
//int pcmcistuple(int, int, int, void*, int);
//PCMmap* pcmmap(int, ulong, int, int);
int pcmspecial(char*, ISAConf*);
//int (*_pcmspecial)(char *, ISAConf *);
//void  pcmspecialclose(int);
//void  (*_pcmspecialclose)(int);
//void  pcmunmap(int, PCMmap*);
int pdmap(ulong*, ulong, ulong, int);
void  procrestore(Proc*);
void  procsave(Proc*);
void  procsetup(Proc*);
//@Scheck: Assembly
void  putcr0(ulong);
//@Scheck: Assembly
void  putcr3(ulong);
//@Scheck: Assembly
void  putcr4(ulong);
kern_addr3 rampage(void);
//@Scheck: Assembly
void  rdmsr(int, vlong*);
//void  realmode(Ureg*);
void  screeninit(void);
void  (*screenputs)(char*, int);
void* sigsearch(char*);
void  syncclock(void);
void  syscallfmt(int syscallno, ulong pc, va_list list);
void  sysretfmt(int syscallno, va_list list, long ret, uvlong start, uvlong stop);
void* tmpmap(Page*);
void  tmpunmap(void*);
//@Scheck: Assembly
void  touser(void*);
void  trapenable(int, void (*)(Ureg*, void*), void*, char*);
void  trapinit(void);
void  trapinit0(void);
ulong upaalloc(int, int);
//void  upafree(ulong, int);
void  upareserve(ulong, int);
/*s: function userureg */
#define userureg(ur) (((ur)->cs & 0xFFFF) == UESEL)
/*e: function userureg */
//@Scheck: Assembly
void  vectortable(void);
void* vmap(ulong, int);
int vmapsync(ulong);
void  vunmap(void*, int);
//@Scheck: Assembly
void  wbinvd(void);
//@Scheck: Assembly
void  wrmsr(int, vlong);
//int xchgw(ushort*, int);

//int iounused(int start, int end); not used anymore in vga.c

/*s: fns.h macros */
/*s: function KADDR */
#define KADDR(pa)  kaddr(pa)
/*e: function KADDR */
/*s: function PADDR */
#define PADDR(ka)  paddr((kern_addr3)(ka))
/*e: function PADDR */

// used in devaudio, maybe could remove it
#define dcflush(a, b)

#define BIOSSEG(a)  KADDR(((uint)(a))<<4)

#define L16GET(p) (((p)[1]<<8)|(p)[0])
#define L32GET(p) (((u32int)L16GET((p)+2)<<16)|L16GET(p))
/*e: fns.h macros */
/*e: fns.h declarations */
/*e: fns.h */
