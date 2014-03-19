
// (*print) is declared lib.h
int		(*iprint)(char*, ...);
int		(*pprint)(char*, ...);

void		(*panic)(char*, ...);
void		(*_assert)(char*);

void		(*dumpstack)(void);
void		(*dumpaproc)(Proc*);

void		(*error)(char*);
void		(*nexterror)(void);

Proc*		(*wakeup)(Rendez*);
void		(*sched)(void);
void		(*ready)(Proc*);
void		(*sleep)(Rendez*, int(*)(void*), void*);
void		(*tsleep)(Rendez*, int (*)(void*), void*, ulong);

Proc*		(*proctab)(int);
int	(*postnote)(Proc*, int, char*, int);
void		(*pexit)(char*, int);

int		(*return0)(void*);

void		(*cclose)(Chan*);

void		(*exit)(int);

void	(*coherence)(void); // was in 386/fns.h

uvlong		(*fastticks)(uvlong*);

//???
void		(*consdebug)(void);
void		(*delay)(int);
void		(*kproftimer)(ulong);
void		(*microdelay)(int);

#include "../port/portfns_concurrency.h"
#include "../port/portfns_memory.h"
#include "../port/portfns_files.h"
#include "../port/portfns_processes.h"
#include "../port/portfns_misc.h"
#include "../port/portfns_console.h"
#include "../port/portfns_buses.h"
#include "../port/portfns_devices.h"

// exported in lib.h, defined in 386/ (but used in port)
//extern	int	cistrcmp(char*, char*);
//extern	int	cistrncmp(char*, char*, int);

//unused: void		addwatchdog(Watchdog*);
void		bootlinks(void);
//unused:long		clrfpintr(void);
void		confinit(void);
//int		decrypt(void*, void*, int);
//int		encrypt(void*, void*, int);
//int		eqchan(Chan*, Chan*, int);
void		flushmmu(void);
void		forkchild(Proc*, Ureg*);
void		forkret(void);
void		getcolor(ulong, ulong*, ulong*, ulong*);
ulong		getmalloctag(void*);
ulong		getrealloctag(void*);
void		gotolabel(Label*);
int		haswaitq(void*);
long		hostdomainwrite(char*, int);
long		hostownerwrite(char*, int);
int		iseve(void);
Segment*	isoverlap(Proc*, ulong, int);
int		ispages(void*);
int		kbdgetmap(uint, int*, int*, Rune*);
void		kbdputmap(ushort, ushort, Rune);
void		kprocchild(Proc*, void (*)(void*), void*);
ulong		l2be(long);
long		latin1(Rune*, int);
void		logopen(Log*);
void		logclose(Log*);
char*		logctl(Log*, int, char**, Logflag*);
void		logn(Log*, int, void*, int);
long		logread(Log*, void*, ulong, long);
void		log(Log*, int, char*, ...);
void		machinit(void);
uvlong		mk64fract(uvlong, uvlong);
void		mmurelease(Proc*);
void		mmuswitch(Proc*);
Chan*		mntauth(Chan*, char*);
long		mntversion(Chan*, char*, int, int);
void		mouseresize(void);
void		mul64fract(uvlong*, uvlong, uvlong);
void		muxclose(Mnt*);
int		newfd(Chan*);
int		notify(Ureg*);
ulong		perfticks(void);
void		pio(Segment *, ulong, ulong, Page **);
#define		poperror()		up->nerrlab--
void		prflush(void);
int		procfdprint(Chan*, int, int, char*, int);
void		putmmu(ulong, ulong, Page*);
int		rand(void);
void		readn(Chan *, void *, long);
int		readnum(ulong, char*, ulong, ulong, int);
int		readstr(ulong, char*, ulong, char*);
void		rebootcmd(int, char**);
void		reboot(void*, void*, ulong);
void		resched(char*);
long		rtctime(void);
Proc*		runproc(void);
void		savefpregs(FPsave*);
int		setcolor(ulong, ulong, ulong, ulong);
void		setkernur(Ureg*, Proc*);
int		setlabel(Label*);
void		setregisters(Ureg*, char*, char*, int);
char*		skipslash(char*);
char*		srvname(Chan*);
void		timerset(Tval);
long		unionread(Chan*, void*, long);
void		userinit(void);
ulong		userpc(void);
long		userwrite(char*, int);
void*		xallocz(ulong, int);
void		xhole(ulong, ulong);
Segment*	data2txt(Segment*);
void		hnputv(void*, uvlong);
void		hnputl(void*, uint);
void		hnputs(void*, ushort);
uvlong		nhgetv(void*);
uint		nhgetl(void*);
ushort		nhgets(void*);
ulong		µs(void);

int		islo(void);

int		splhi(void);
int		spllo(void);
void		splx(int);
void		splxpc(int);

//tas
void		_xinc(long*);
long		_xdec(long*);

long		lcycles(void);

#pragma varargck argpos iprint	1
#pragma	varargck argpos	panic	1
#pragma varargck argpos pprint	1
