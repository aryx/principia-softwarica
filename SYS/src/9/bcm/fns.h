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

Dirtab*	addarchfile(char*, int, long(*)(Chan*, void*, long, vlong), 
	long(*)(Chan*, void*, long, vlong));

extern void archreboot(void);
extern void archreset(void);
extern void armtimerset(int);

extern void cachedwb(void);
extern void cachedwbinv(void);
extern void cachedinvse(void*, int);
extern void cachedwbse(void*, int);
extern void cachedwbinvse(void*, int);
extern void cacheiinv(void);
extern void cacheuwbinv(void);

extern uintptr cankaddr(uintptr pa);

extern int cas32(void*, u32int, u32int);
extern int cas(ulong*, ulong, ulong);
extern int cmpswap(long*, long, long);

extern void checkmmu(uintptr, uintptr);

extern void clockinit(void);
extern void clockshutdown(void);

//extern void coherence(void);
extern void (*coherence)(void);
extern void coherence1(void);

extern u32int cpidget(void);
extern ulong cprd(int cp, int op1, int crn, int crm, int op2);
extern ulong cprdsc(int op1, int crn, int crm, int op2);
extern void cpwr(int cp, int op1, int crn, int crm, int op2, ulong val);
extern void cpwrsc(int op1, int crn, int crm, int op2, ulong val);

extern void cpuidprint(void);
extern char *cputype2name(char *buf, int size);


// was in portfns.h but commented because not used in x86
long    lcycles(void);
#define cycles(ip) *(ip) = lcycles()

// could be in portfns.h, same type in every arch, but called from arch-specific
void    forkret(void);
void		dumpregs(Ureg*);
void		confinit(void);
void		printinit(void);
void		userinit(void);
void		bootlinks(void);
void  memorysummary(void);

// lib/latin1.c
long    latin1(Rune*, int);


extern uintptr dmaaddr(void *va);
extern void dmastart(int, int, int, void*, void*, int);
extern int dmawait(int);

extern int fbblank(int);
extern void* fbinit(int, int*, int*, int*);

extern u32int farget(void);

extern void fpon(void);
extern ulong fprd(int fpreg);
extern void fprestreg(int fpreg, uvlong val);
extern void fpsave(ArchFPsave *);
extern ulong fpsavereg(int fpreg, uvlong *fpp);
extern void fpwr(int fpreg, ulong val);
extern u32int fsrget(void);

extern uint getboardrev(void);
extern ulong getclkrate(int);

extern char* getconf(char*);
extern uint getcputemp(void);

extern char *getethermac(void);
extern uint getfirmware(void);

extern int getncpus(void);
extern int getpower(int);

extern void getramsize(Confmem*);

extern void gpiosel(uint, int);
extern void gpiopullup(uint);
extern void gpiopulloff(uint);
extern void gpiopulldown(uint);

extern void i2cread(uint, void*, int);
extern void i2cwrite(uint, void*, int);

extern u32int ifsrget(void);

extern void irqenable(int, void (*)(Ureg*, void*), void*);
#define intrenable(i, f, a, b, n) irqenable((i), (f), (a))


extern void intrcpushutdown(void);
extern void intrshutdown(void);
extern void intrsoff(void);

extern int isaconfig(char*, int, ISAConf*);

extern int l2ap(int);
extern void l2cacheuwbinv(void);

extern void links(void);

extern void mmuinit(void*);
extern void mmuinit1(void*);
extern void mmuinvalidate(void);
extern void mmuinvalidateaddr(u32int);
extern uintptr mmukmap(uintptr, uintptr, usize);
extern void okay(int);
//extern void procrestore(Proc *);
//extern void procsave(Proc*);
//extern void procsetup(Proc*);

extern void screeninit(void);

#define sdfree(p) free(p)
#define sdmalloc(n)	mallocalign(n, BLOCKALIGN, 0, 0)

extern void setclkrate(int, ulong);
extern void setpower(int, int);
extern void setr13(int, u32int*);

extern void spirw(uint, void*, int);
extern int splfhi(void);
extern int splflo(void);

extern void swcursorinit(void);

extern void syscallfmt(int syscallno, ulong pc, va_list list);
extern void sysretfmt(int syscallno, va_list list, long ret, uvlong start, uvlong stop);

extern int startcpus(uint);
extern void stopcpu(uint);

extern int tas(void *);

extern void touser(uintptr);

extern void trapinit(void);

extern void uartconsinit(void);
extern int userureg(Ureg*);
extern void vectors(void);
extern void vtable(void);

extern void wdogoff(void);

/*
 * floating point emulation
 */
extern int fpiarm(Ureg*);
extern int fpudevprocio(Proc*, void*, long, uintptr, int);
extern void fpuinit(void);
extern void fpunoted(void);
extern void fpunotify(Ureg*);
extern void fpuprocrestore(Proc*);
extern void fpuprocsave(Proc*);
extern void fpusysprocsetup(Proc*);
extern void fpusysrfork(Ureg*);
extern void fpusysrforkchild(Proc*, Ureg*, Proc*);
extern int fpuemu(Ureg*);

/*
 * Miscellaneous machine dependent stuff.
 */
extern char* getenv(char*, char*, int);
uintptr mmukmap(uintptr, uintptr, usize);
uintptr mmukunmap(uintptr, uintptr, usize);
extern void* mmuuncache(void*, usize);
extern void* ucalloc(usize);
extern Block* ucallocb(int);
extern void* ucallocalign(usize size, int align, int span);
extern void ucfree(void*);
extern void ucfreeb(Block*);

/*
 * Things called from port.
 */
extern void delay(int);				/* only scheddump() */
extern int islo(void);
extern void microdelay(int);			/* only edf.c */
extern void idlehands(void);
extern void setkernur(Ureg*, Proc*);		/* only devproc.c */
extern void* sysexecregs(uintptr, ulong, int);
extern void sysprocsetup(Proc*);
extern void validalign(uintptr, unsigned);

extern void kexit(Ureg*);

#define	getpgcolor(a)	0
#define	kmapinval()
#define countpagerefs(a, b)

#define PTR2UINT(p)	((uintptr)(p))
#define UINT2PTR(i)	((void*)(i))

//#define	waserror()	(up->nerrlab++, setlabel(&up->errlab[up->nerrlab-1]))

#define KADDR(pa)	UINT2PTR(KZERO    | ((uintptr)(pa) & ~KSEGM))
#define PADDR(va)	PTR2UINT(PHYSDRAM | ((uintptr)(va) & ~KSEGM))

#define MASK(v)	((1UL << (v)) - 1)	/* mask `v' bits wide */
