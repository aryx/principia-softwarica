
//some kind of #include "../port/portfns_core.h"

// exported in lib.h, defined in portfns.c
// (*print) is declared lib.h

// exported in lib.h, defined in 386/fns.c (but used in port)
// int	cistrcmp(char*, char*);
// int	cistrncmp(char*, char*, int);

// portfns.c (mostly here just to remove some backward dependencies)
// console/devcons.c
int		(*iprint)(char*, ...);
int		(*pprint)(char*, ...);
// (*print)
void		(*panic)(char*, ...);
void		(*_assert)(char*);
// process/386/trap.c
void		(*dumpstack)(void);
// process/proc.c
void		(*dumpaproc)(Proc*);
// process/proc.c
void		(*error)(char*);
void		(*nexterror)(void);
// process/proc.c
Proc*		(*wakeup)(Rendez*);
void		(*sched)(void);
void		(*ready)(Proc*);
void		(*sleep)(Rendez*, int(*)(void*), void*);
void		(*tsleep)(Rendez*, int (*)(void*), void*, ulong);
// process/proc.c
Proc*		(*proctab)(int);
int	(*postnote)(Proc*, int, char*, int);
void		(*pexit)(char*, int);
//process/sysproc.c
int		(*return0)(void*);
// files/chan.c
void		(*cclose)(Chan*);
// init/main.c
void		(*exit)(int);
//TODO?char* (*getconf)(char *name) = 0;
//misc/386/devarch.c
void	(*coherence)(void);
//misc/386/devarch.c
uvlong		(*fastticks)(uvlong*);
// processes/386/i8253.c
void		(*microdelay)(int);
void		(*delay)(int);

// portfns.c
int		readnum(ulong, char*, ulong, ulong, int);
int		readstr(ulong, char*, ulong, char*);

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

//in init/386/main.c (but used in port), TODO should be in portfns_init.h
void		reboot(void*, void*, ulong);

// unused and undefined
//void		addwatchdog(Watchdog*);
//long		clrfpintr(void);
//int		decrypt(void*, void*, int);
//int		encrypt(void*, void*, int);
//int		eqchan(Chan*, Chan*, int);
//void		logopen(Log*);
//void		logclose(Log*);
//char*		logctl(Log*, int, char**, Logflag*);
//void		logn(Log*, int, void*, int);
//long		logread(Log*, void*, ulong, long);
//void		log(Log*, int, char*, ...);
//void		resched(char*);
//void		savefpregs(FPsave*);

#pragma varargck argpos iprint	1
#pragma varargck argpos pprint	1
#pragma	varargck argpos	panic	1
