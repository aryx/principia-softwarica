/*s: portfns_core.h */

// exported in lib.h, defined in portfns.c
// (*print)

// exported in lib.h, defined in 386/fns.c (but used in port)
// int  cistrcmp(char*, char*);
// int  cistrncmp(char*, char*, int);

// could be in lib.h
/*s: portdat.h macros */
#define ROUND(s, sz)  (((s)+(sz-1)) & ~(sz-1))
/*x: portdat.h macros */
#define MASK(v) ((1UL << (v)) - 1)  /* mask `v' bits wide */
/*x: portdat.h macros */
#define MIN(a, b) ((a) < (b)? (a): (b))
/*x: portdat.h macros */
// used in bcm/mmu.c
#define FEXT(d, o, w) (((d)>>(o)) & ((1<<(w))-1)) 
/*x: portdat.h macros */
#define HOWMANY(x, y) (((x)+((y)-1))/(y))
#define ROUNDUP(x, y) (HOWMANY((x), (y))*(y)) /* ceiling */
/*x: portdat.h macros */
// BY2PG is defined in mem.h, which should always be included before "dat.h"!
#define PGROUND(s)  ROUNDUP(s, BY2PG)
/*e: portdat.h macros */

// portfns.c (mostly here just to remove some backward dependencies)
/*s: portfns_core.h backward deps breaker */
// console/devcons.c
int   (*iprint)(char*, ...);
int   (*pprint)(char*, ...);
void  (*panic)(char*, ...);
void  (*_assert)(char*);
/*x: portfns_core.h backward deps breaker */
// processes/<arch>/trap.c
void    (*arch_dumpstack)(void);
/*x: portfns_core.h backward deps breaker */
// processes/proc.c
void    (*dumpaproc)(Proc*);
/*x: portfns_core.h backward deps breaker */
// processes/proc.c
void    (*error)(char*);
void    (*nexterror)(void);
/*x: portfns_core.h backward deps breaker */
// processes/proc.c
void    (*sleep)(Rendez*, int(*)(void*), void*);
void    (*tsleep)(Rendez*, int (*)(void*), void*, ulong);
Proc*   (*wakeup)(Rendez*);
void    (*sched)(void);
void    (*ready)(Proc*);
/*x: portfns_core.h backward deps breaker */
// processes/proc.c
Proc*   (*proctab)(int);
int     (*postnote)(Proc*, int, char*, int);
void    (*pexit)(char*, bool);
/*x: portfns_core.h backward deps breaker */
// files/chan.c
void    (*cclose)(Chan*);
/*x: portfns_core.h backward deps breaker */
// init/<arch>/main.c
void    (*arch_exit)(int);
/*x: portfns_core.h backward deps breaker */
//misc/<arch>/devarch.c
void    (*arch_coherence)(void);
uvlong  (*arch_fastticks)(uvlong*);
/*x: portfns_core.h backward deps breaker */
// time/<arch>/time.c
void    (*arch_microdelay)(int);
void    (*arch_delay)(int);
// init/<arch>/main.c
bool (*arch_isaconfig)(char*, int, ISAConf*);
/*e: portfns_core.h backward deps breaker */

// portfns.c
bool returnfalse(void*);
int   readnum(ulong, char*, ulong, ulong, int);
int   readstr(ulong, char*, ulong, char*);

/*s: portfns_core.h pragmas */
#pragma varargck argpos iprint  1
#pragma varargck argpos pprint  1
#pragma varargck argpos panic 1
/*e: portfns_core.h pragmas */
/*e: portfns_core.h */
