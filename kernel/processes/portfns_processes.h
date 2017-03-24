/*s: portfns_processes.h */

// proc.c
// (now in portfns_core.h, to remove some backward dependencies)
//void  (*sched)(void);
//void  (*sleep)(Rendez*, int(*)(void*), void*);
//void  (*tsleep)(Rendez*, int (*)(void*), void*, ulong);
//Proc* (*wakeup)(Rendez*);
//int   (*postnote)(Proc*, int, char*, int);
//void  (*error)(char*);
//void  (*nexterror)(void);
//void  (*pexit)(char*, bool);
//void  (*ready)(Proc*);
//void  (*dumpaproc)(Proc*);
//
void    yield(void);
void    hzsched(void);
void    schedinit(void);
Proc*   newproc(void);
void    preempt(void);
void    procinit(void);
int     procindex(ulong);
void    kproc(char*, void(*)(void*), void*);
Proc*   dequeueproc(Schedq*, Proc*);
//
void    scheddump(void);
void    procdump(void);
//
void    exhausted(char*);
void    procctl(Proc*);
void    procwired(Proc*, int);
void    procpriority(Proc*, int, int);
void    accounttime(void);
int     canpage(Proc*);
int     anyhigher(void);
void    killbig(char*);
void    procflushseg(Segment*); 
void    renameuser(char*, char*);
ulong   pwait(Waitmsg*);
void    unbreak(Proc*);
int     freebroken(void);
int     haswaitq(void*);

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
Mount*  newmount(Chan*, int, char*);
void    resrcwait(char*);


// edf.c (used to be declared in edf.h)
Edf*    edflock(Proc*);
void    edfunlock(void);
void    edfrun(Proc*, int);
void    edfinit(Proc*);
void    edfrecord(Proc*);
void    edfstop(Proc*);
void    edfyield(void);
int     edfready(Proc*);
char*   edfadmit(Proc*);

// sysproc.c
// not useless, used by misc/rebootcmd.c, weird not recognized by cg
ulong   l2be(long);
// many sysxxx functions (used in syscalls/ without requiring extern decl)


// <arch>/trap.c (but used in port)
int     arch_userureg(Ureg *);
void    arch_callwithureg(void(*)(Ureg*));
ulong   arch_dbgpc(Proc*);
ulong   arch_userpc(void);
long    arch_execregs(ulong, ulong, ulong);
void    arch_forkchild(Proc*, Ureg*);
void    arch_kprocchild(Proc*, void (*)(void*), void*);
void    arch_setregisters(Ureg*, char*, char*, int);
void    arch_setkernur(Ureg*, Proc*);
void    arch_validalign(uintptr, unsigned); // TODO: should be in memory/?

// in <arch>/forkret.s, called from arch_forkchild
//@Scheck: Assembly
void  arch__forkret(void); 


// <arch>/main_processes.c (but used in port)
void  arch_procsetup(Proc*);
void  arch_procsave(Proc*);
void  arch_procrestore(Proc *);
void  arch_idlehands(void);


// in <arch>/l.s (but used in port)
//@Scheck: Assembly
void  arch_gotolabel(Label*);
//@Scheck: Assembly
int   arch_setlabel(Label*);
//@Scheck: Assembly
void  arch_mul64fract(uvlong*, uvlong, uvlong);

// portdat_processes.c
void (*proctrace)(Proc*, int, vlong); // was in devproc.c
void (*kproftimer)(ulong); // was in portfns.h
/*e: portfns_processes.h */
