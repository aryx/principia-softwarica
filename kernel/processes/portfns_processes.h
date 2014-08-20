/*s: portfns_processes.h */

// tod.c
// initialize the hidden global tod.c#tod
void    todinit(void);
void    todsetfreq(vlong);
void    todset(vlong, vlong, int);
vlong   todget(vlong*);
//uvlong    (*fastticks)(uvlong*); is in 386/devarch.c (but used in port)
uvlong    fastticks2us(uvlong);
uvlong    ns2fastticks(uvlong);
long    seconds(void);
//uvlong    tod2fastticks(vlong);
//uvlong    us2fastticks(uvlong);
//uvlong    ms2fastticks(ulong);
//uvlong    fastticks2ns(uvlong);
//todfix(): TODO false positive, static forward decl

// pgrp.c
Rgrp*   newrgrp(void);
Pgrp*   newpgrp(void);
void    closergrp(Rgrp*);
void    closefgrp(Fgrp*);
Fgrp*   dupfgrp(Fgrp*);
void    closepgrp(Pgrp*);
void    pgrpcpy(Pgrp*, Pgrp*);
void    forceclosefgrp(void);
void    pgrpnote(ulong, char*, long, int);
void    mountfree(Mount*);
Mount*    newmount(Chan*, int, char*);
void    resrcwait(char*);

// portclock.c
ulong   tk2ms(ulong);
#define   TK2MS(x) ((x)*(1000/HZ))
#define   MS2NS(n) (((vlong)(n))*1000000LL)
ulong   ms2tk(ulong);
void    timerdel(Timer*);
void    timeradd(Timer*);
Timer*    addclock0link(void (*)(void), int);
void    timerintr(Ureg*, Tval);
void    timersinit(void);
//void    timerset(Tval); is in 386/devarch.c (but used in port)

// proc.c
void exhausted(char*);
// in portfns_core.h, to remove some backward dependencies
//void    (*sleep)(Rendez*, int(*)(void*), void*);
//int (*postnote)(Proc*, int, char*, int);
void    procctl(Proc*);
void    procwired(Proc*, int);
//Proc*   (*wakeup)(Rendez*);
//void    (*error)(char*);
void    procpriority(Proc*, int, int);
//TODO: fp non-deps twakeup, passed as pointer func
//void    (*nexterror)(void);
//void    (*dumpaproc)(Proc*);
void    accounttime(void);
int   canpage(Proc*);
int   anyhigher(void);
void    scheddump(void);
Proc*   dequeueproc(Schedq*, Proc*);
void   preempt(void);
void    killbig(char*);
void    procflushseg(Segment*); 
void    procdump(void);
void    renameuser(char*, char*);
void    procinit(void);
int   procindex(ulong);
ulong   pwait(Waitmsg*);
//void    (*tsleep)(Rendez*, int (*)(void*), void*, ulong);
void    unbreak(Proc*);
int   freebroken(void);
void    yield(void);
//void    (*pexit)(char*, bool);
//void    (*sched)(void);
void    schedinit(void);
//void    (*ready)(Proc*);
Proc*   newproc(void);
void    kproc(char*, void(*)(void*), void*);
void    hzsched(void);
//not used outside: int   anyready(void);
//TODO cg didn't find ref outside in devproc.c?
int   haswaitq(void*);

// alarm.c
ulong   procalarm(ulong);
void    alarmkproc(void*);
void    checkalarms(void);

// edf.c (used to be declared in edf.h)
Edf*    edflock(Proc*);
void    edfunlock(void);
//TODO fp deadlineintr, static func passed via pointer again
void    edfrun(Proc*, int);
void    edfinit(Proc*);
void    edfrecord(Proc*);
void    edfstop(Proc*);
void    edfyield(void);
int   edfready(Proc*);
char*   edfadmit(Proc*);

// sysproc.c
// not useless, used by misc/rebootcmd.c, weird not recognized by cg
ulong   l2be(long);
// many sysxxx functions (used in syscalls/ without requiring extern decl)




// 386/trap.c (but used in port)
void    callwithureg(void(*)(Ureg*));
ulong   dbgpc(Proc*);
long    execregs(ulong, ulong, ulong);
void    forkchild(Proc*, Ureg*);
ulong   userpc(void);
void    setregisters(Ureg*, char*, char*, int);
void    setkernur(Ureg*, Proc*);
void    kprocchild(Proc*, void (*)(void*), void*);
// intrenable(), but mostly used in 386, just in port/devaudio.c

// 386/main_processes.c (but used in port)
void procsetup(Proc*);
void procsave(Proc*);
void procrestore(Proc *);
void idlehands(void);

// 386/i8253.c (but used in port)
ulong   perfticks(void);

// 386/devarch.c (but used in port)
void    timerset(Tval);
ulong   us(void);

// in 386/l.s (but used in port)
//@Scheck: Assembly
void    gotolabel(Label*);
//@Scheck: Assembly
int   setlabel(Label*);
//@Scheck: Assembly
void    mul64fract(uvlong*, uvlong, uvlong);

// portdat_processes.c
void (*proctrace)(Proc*, int, vlong); // was in devproc.c
void (*kproftimer)(ulong); // was in portfns.h
/*e: portfns_processes.h */
