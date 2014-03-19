
// tod.c
void    todinit(void);
void    todsetfreq(vlong);
void    todset(vlong, vlong, int);
vlong   todget(vlong*);
//uvlong    (*fastticks)(uvlong*); is in devarch
//uvlong    tod2fastticks(vlong);
uvlong    fastticks2us(uvlong);
//uvlong    us2fastticks(uvlong);
//uvlong    ms2fastticks(ulong);
uvlong    ns2fastticks(uvlong);
//uvlong    fastticks2ns(uvlong);
long    seconds(void);
//todfix: TODO false positive, static forward decl

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
Mount*    newmount(Mhead*, Chan*, int, char*);
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
//void    timerset(Tval); in devarch

// proc.c
void exhausted(char*);
//void    (*sleep)(Rendez*, int(*)(void*), void*); proc_sleep
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
int   preempted(void);
void    killbig(char*);
void    procflushseg(Segment*); //TODO: fp non-deps
void    procdump(void);
void    renameuser(char*, char*);
void    procinit0(void);
int   procindex(ulong);
ulong   pwait(Waitmsg*);
//void    (*tsleep)(Rendez*, int (*)(void*), void*, ulong);
void    unbreak(Proc*);
int   freebroken(void);
void    yield(void);
//void    (*pexit)(char*, int);
//void    (*sched)(void);
void    schedinit(void);
//void    (*ready)(Proc*);
Proc*   newproc(void);
void    kproc(char*, void(*)(void*), void*);
void    hzsched(void);
//not used outside: int   anyready(void);

// alarm.c
ulong   procalarm(ulong);
void    alarmkproc(void*);
void    checkalarms(void);

// edf.c
// used to be in edf.h
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
// many sysxxx functions


// 386/trap.c (but used in port)
void    callwithureg(void(*)(Ureg*));
ulong   dbgpc(Proc*);
long    execregs(ulong, ulong, ulong);

// 386/main_processes.c (but used in port)
void procsetup(Proc*);
void procsave(Proc*);
void procrestore(Proc *);
void idlehands(void);
