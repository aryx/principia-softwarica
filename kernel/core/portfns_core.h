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
#define MIN(a, b) ((a) < (b)? (a): (b))
#define HOWMANY(x, y) (((x)+((y)-1))/(y))
#define ROUNDUP(x, y) (HOWMANY((x), (y))*(y)) /* ceiling */

// BY2PG is defined in mem.h, which should always be included before "dat.h"!
#define PGROUND(s)  ROUNDUP(s, BY2PG)

#define FEXT(d, o, w) (((d)>>(o)) & ((1<<(w))-1)) // used in bcm/mmu.c
/*e: portdat.h macros */

// portfns.c (mostly here just to remove some backward dependencies)
/*s: portfns_core.h backward deps breaker */
int devcons_print(char*, ...);
int devcons_pprint(char*, ...);
void devcons_panic(char*, ...);
void devcons__assert(char*);
void trap_dumpstack(void);
void proc_dumpaproc(Proc *p);
void proc_error(char*);
void proc_nexterror(void);
void i8253_delay(int millisecs);
void i8253_microdelay(int microsecs);
//void proc_sched(void);
void sched(void);
void proc_ready(Proc*);
void proc_sleep(Rendez*, int(*)(void*), void*);
void proc_tsleep(Rendez *r, int (*fn)(void*), void *arg, ulong ms);
Proc* proc_wakeup(Rendez*);
void proc_pexit(char *exitstr, bool freemem);
Proc* proc_proctab(int i);
int proc_postnote(Proc *p, int dolock, char *n, int flag);
void chan_cclose(Chan *c);
void main_exit(int ispanic);
int  main_isaconfig(char *class, int ctlrno, ISAConf *isa);
void nop(void);
uvlong devarch_fastticks(uvlong *hz);
void devarch_hook_ioalloc();

void    (*coherence)(void);
int   (*iprint)(char*, ...);
int devcons_iprint(char*, ...);
/*x: portfns_core.h backward deps breaker */
#define print         devcons_print
//#define iprint        devcons_iprint
#define pprint        devcons_pprint
#define panic         devcons_panic
#define _assert       devcons__assert
#define error         proc_error
#define nexterror     proc_nexterror
#define dumpstack     trap_dumpstack
#define dumpaproc     proc_dumpaproc
//#define devtab        conf_devtab
#define delay         i8253_delay
#define microdelay    i8253_microdelay
#define wakeup        proc_wakeup
//#define sched()         proc_sched()
#define ready         proc_ready
#define sleep         proc_sleep
#define tsleep        proc_tsleep
#define exit          main_exit
#define isaconfig     main_isaconfig
//#define coherence     nop
#define fastticks     devarch_fastticks
#define cclose        chan_cclose
#define proctab       proc_proctab
#define postnote      proc_postnote
#define pexit         proc_pexit
//#define hook_ioalloc  devarch_hook_ioalloc
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
