/*s: syscalls/arm/syscall.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */
#include "../port/systab.h"
#include <tos.h>
#include "ureg.h"
#include "arm.h"

typedef struct NFrame NFrame;
/*s: struct NFrame(arm) */
struct NFrame {
    uintptr ip;
    Ureg*   arg0;
    char*   arg1;
    char    msg[ERRMAX];
    Ureg*   old;
    Ureg    ureg;
};
/*e: struct NFrame(arm) */

/*s: function arch__noted(arm) */
/*
 *   Return user to state before notify()
 */
static void
arch__noted(Ureg* cur, uintptr arg0)
{
    NFrame *nf;
    Ureg *nur;

    qlock(&up->debug);
    if(arg0 != NRSTR && !up->notified){
        qunlock(&up->debug);
        pprint("call to noted() when not notified\n");
        pexit("Suicide", 0);
    }
    up->notified = 0;
    fpunoted();

    nf = up->ureg;

    /* sanity clause */
    if(!okaddr(PTR2UINT(nf), sizeof(NFrame), 0)){
        qunlock(&up->debug);
        pprint("bad ureg in noted %#p\n", nf);
        pexit("Suicide", 0);
    }

    /* don't let user change system flags */
    nur = &nf->ureg;
    nur->psr &= PsrMask|PsrDfiq|PsrDirq;
    nur->psr |= (cur->psr & ~(PsrMask|PsrDfiq|PsrDirq));

    memmove(cur, nur, sizeof(Ureg));

    switch((int)arg0){
    case NCONT:
    case NRSTR:
        if(!okaddr(nur->pc, BY2WD, 0) || !okaddr(nur->sp, BY2WD, 0)){
            qunlock(&up->debug);
            pprint("suicide: trap in noted\n");
            pexit("Suicide", 0);
        }
        up->ureg = nf->old;
        qunlock(&up->debug);
        break;
    case NSAVE:
        if(!okaddr(nur->pc, BY2WD, 0) || !okaddr(nur->sp, BY2WD, 0)){
            qunlock(&up->debug);
            pprint("suicide: trap in noted\n");
            pexit("Suicide", 0);
        }
        qunlock(&up->debug);

        arch_splhi();
        nf->arg1 = nf->msg;
        nf->arg0 = &nf->ureg;
        nf->ip = 0;
        cur->sp = PTR2UINT(nf);
        cur->r0 = PTR2UINT(nf->arg0);
        break;
    default:
        pprint("unknown noted arg %#p\n", arg0);
        up->lastnote.flag = NDebug;
        /*FALLTHROUGH*/
    case NDFLT:
        if(up->lastnote.flag == NDebug){ 
            qunlock(&up->debug);
            pprint("suicide: %s\n", up->lastnote.msg);
        }
        else
            qunlock(&up->debug);
        pexit(up->lastnote.msg, up->lastnote.flag != NDebug);
    }
}
/*e: function arch__noted(arm) */

/*s: function arch__notify(arm) */
/*
 *  Call user, if necessary, with note.
 *  Pass user the Ureg struct and the note on his stack.
 */
int
arch__notify(Ureg* ureg)
{
    int l;
    Note *n;
    u32int s;
    uintptr sp;
    NFrame *nf;

    if(up->procctl)
        procctl(up);
    if(up->nnote == 0)
        return 0;

    fpunotify(ureg);

    s = arch_spllo();
    qlock(&up->debug);

    up->notepending = 0;
    n = &up->note[0];
    if(strncmp(n->msg, "sys:", 4) == 0){
        l = strlen(n->msg);
        if(l > ERRMAX-23)   /* " pc=0x0123456789abcdef\0" */
            l = ERRMAX-23;
        snprint(n->msg + l, sizeof n->msg - l, " pc=%#lux", ureg->pc);
    }

    if(n->flag != NUser && (up->notified || up->notify == 0)){
        if(n->flag == NDebug)
            pprint("suicide: %s\n", n->msg);
        qunlock(&up->debug);
        pexit(n->msg, n->flag != NDebug);
    }

    if(up->notified){
        qunlock(&up->debug);
        arch_splhi();
        return 0;
    }
        
    if(up->notify == nil){
        qunlock(&up->debug);
        pexit(n->msg, n->flag != NDebug);
    }
    if(!okaddr(PTR2UINT(up->notify), 1, 0)){
        pprint("suicide: notify function address %#p\n", up->notify);
        qunlock(&up->debug);
        pexit("Suicide", 0);
    }

    sp = ureg->sp - sizeof(NFrame);
    if(!okaddr(sp, sizeof(NFrame), 1)){
        qunlock(&up->debug);
        pprint("suicide: notify stack address %#p\n", sp);
        pexit("Suicide", 0);
    }

    nf = UINT2PTR(sp);
    memmove(&nf->ureg, ureg, sizeof(Ureg));
    nf->old = up->ureg;
    up->ureg = nf;
    memmove(nf->msg, up->note[0].msg, ERRMAX);
    nf->arg1 = nf->msg;
    nf->arg0 = &nf->ureg;
    ureg->r0 = PTR2UINT(nf->arg0);
    nf->ip = 0;

    ureg->sp = sp;
    ureg->pc = PTR2UINT(up->notify);
    up->notified = 1;
    up->nnote--;
    memmove(&up->lastnote, &up->note[0], sizeof(Note));
    memmove(&up->note[0], &up->note[1], up->nnote*sizeof(Note));

    qunlock(&up->debug);
    arch_splx(s);

    return 1;
}
/*e: function arch__notify(arm) */

/*s: function arch__syscall(arm) */
void
arch__syscall(Ureg* ureg)
{
    int scallnr;
    long ret;
    ulong sp; // user_addr
    /*s: [[syscall()]] other locals(arm) */
    char *e;
    u32int s;
    int i;
    /*x: [[syscall()]] other locals(arm) */
    vlong startns, stopns;
    /*e: [[syscall()]] other locals(arm) */

    /*s: [[syscall()]] sanity check call from userspace(arm) */
    if(!arch_userureg(ureg))
        panic("syscall: from kernel: pc %#lux r14 %#lux psr %#lux",
            ureg->pc, ureg->r14, ureg->psr);
    /*e: [[syscall()]] sanity check call from userspace(arm) */
    /*s: [[syscall()]] adjust kentry */
    arch_cycles(&up->kentry);
    /*e: [[syscall()]] adjust kentry */

    cpu->syscall++;
    up->insyscall = true;

    up->pc = ureg->pc;
    up->dbgreg = ureg;

    // syscall number!
    scallnr = ureg->r0;
    //up->scallnr = scallnr;

    /*s: [[syscall()]] if RFORK, handle floating point(arm) */
    if(scallnr == RFORK)
        fpusysrfork(ureg);
    /*e: [[syscall()]] if RFORK, handle floating point(arm) */

    arch_spllo();
    sp = ureg->sp;

    /*s: [[syscall()]] Proc_tracesyscall if, syscall entry(arm) */
    if(up->procctl == Proc_tracesyscall){
        /*
         * Redundant validaddr.  Do we care?
         * Tracing syscalls is not exactly a fast path...
         * Beware, validaddr currently does a pexit rather
         * than an error if there's a problem; that might
         * change in the future.
         */
        if(sp < (USTKTOP-BY2PG) || sp > (USTKTOP-sizeof(Sargs)-BY2WD))
            validaddr(sp, sizeof(Sargs)+BY2WD, false);

        syscallfmt(scallnr, ureg->pc, (va_list)(sp+BY2WD));
        up->procctl = Proc_stopme;
        // this will call sched() and wakeup the tracer process
        procctl(up);
        // back here when the tracer process readied us back and
        // should have set procctl back to Proc_tracesyscall
        if(up->syscalltrace) 
            free(up->syscalltrace);
        up->syscalltrace = nil;
    }
    /*e: [[syscall()]] Proc_tracesyscall if, syscall entry(arm) */

    up->nerrlab = 0;
    ret = -1;

    startns = todget(nil);
    if(!waserror()){
        /*s: [[syscall()]] sanity check scallnr(arm) */
        if(scallnr >= nsyscall){
            pprint("bad sys call number %d pc %#lux\n",
                scallnr, ureg->pc);
            postnote(up, 1, "sys: bad sys call", NDebug);
            error(Ebadarg);
        }
        /*e: [[syscall()]] sanity check scallnr(arm) */
        /*s: [[syscall()]] sanity check sp(arm) */
        if(sp < (USTKTOP-BY2PG) || sp > (USTKTOP-sizeof(Sargs)-BY2WD))
            validaddr(sp, sizeof(Sargs)+BY2WD, 0);
        /*e: [[syscall()]] sanity check sp(arm) */

        // copy syscall arguments from user stack to up->sargs
        up->sargs = *((Sargs*)(sp+BY2WD)); // extra BY2WD for?
        up->psstate = sysctab[scallnr];

        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //IMPORTANT: The actual system call
        ret = systab[scallnr](up->sargs.args);
        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        poperror();
    }else{
        /* failure: save the error buffer for errstr */
        e = up->syserrstr;
        up->syserrstr = up->errstr;
        up->errstr = e;
    }
    /*s: [[syscall()]] if nerrlab(arm) */
    if(up->nerrlab){
        print("bad errstack [%d]: %d extra\n", scallnr, up->nerrlab);
        for(i = 0; i < NERR; i++)
            print("sp=%#p pc=%#p\n",
                up->errlab[i].sp, up->errlab[i].pc);
        panic("error stack");
    }
    /*e: [[syscall()]] if nerrlab(arm) */

    /*
     *  Put return value in frame.  On the x86 the syscall is
     *  just another trap and the return value from syscall is
     *  ignored.  On other machines the return value is put into
     *  the results register by caller of syscall.
     */
    ureg->r0 = ret;

    /*s: [[syscall()]] Proc_tracesyscall if, syscall exit(arm) */
    if(up->procctl == Proc_tracesyscall){
        stopns = todget(nil);
        up->procctl = Proc_stopme;
        sysretfmt(scallnr, (va_list)(sp+BY2WD), ret, startns, stopns);
        s = arch_splhi();
        procctl(up); // again, will call sched() and wakeup tracer process
        arch_splx(s);
        if(up->syscalltrace)
            free(up->syscalltrace);
        up->syscalltrace = nil;
    }
    /*e: [[syscall()]] Proc_tracesyscall if, syscall exit(arm) */

    up->insyscall = false;
    up->psstate = nil;

    /*s: [[syscall()]] call noted() */
    if(scallnr == NOTED)
        arch__noted(ureg, *(ulong*)(sp+BY2WD));
    /*e: [[syscall()]] call noted() */
    /*s: [[syscall()]] call notify(arm) */
    arch_splhi();
    if(scallnr != RFORK && (up->procctl || up->nnote))
        arch__notify(ureg);
    /*e: [[syscall()]] call notify(arm) */

    /*s: [[syscall()]] if delaysched(arm) */
    /* if we delayed sched because we held a lock, sched now */
    if(up->delaysched){
        sched();
        arch_splhi();
    }
    /*e: [[syscall()]] if delaysched(arm) */
    arch__kexit(ureg);
}
/*e: function arch__syscall(arm) */

/*s: function arch_execregs(arm) */
long
arch_execregs(ulong entry, ulong ssize, ulong nargs)
{
    ulong *sp;
    Ureg *ureg;

    sp = (ulong*)(USTKTOP - ssize);
    *--sp = nargs;

    ureg = up->dbgreg;
    ureg->r13 = (ulong)sp;
    ureg->pc = entry;

    /*
     * return the address of kernel/user shared data
     * (e.g. clock stuff)
     */
    return USTKTOP-sizeof(Tos);
}
/*e: function arch_execregs(arm) */

/*s: function arch_forkchild(arm) */
/* 
 *  Craft a return frame which will cause the child to pop out of
 *  the scheduler in user mode with the return register zero.  Set
 *  pc to point to a l.s return function.
 */
void
arch_forkchild(Proc *p, Ureg *ureg)
{
    Ureg *cureg;

    p->sched.sp = (ulong)p->kstack+KSTACK-sizeof(Ureg);
    p->sched.pc = (ulong)arch__forkret;

    cureg = (Ureg*)(p->sched.sp);
    memmove(cureg, ureg, sizeof(Ureg));

    /* syscall returns 0 for child */
    cureg->r0 = 0;

    /* Things from bottom of syscall which were never executed */
    p->psstate = nil;
    p->insyscall = false;

    /*s: [[arch_forkchild()]] floating point(arm) */
    fpusysrforkchild(p, cureg, up);
    /*e: [[arch_forkchild()]] floating point(arm) */
}
/*e: function arch_forkchild(arm) */
/*e: syscalls/arm/syscall.c */
