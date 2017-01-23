/*s: sysproc.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

#include    <tos.h>
#include    <a.out.h>

/*s: enum rfork */
//coupling: with libc.h
enum rfork
{
    RFPROC      = (1<<4), // fork a new process!! (if unset then set props for up)
    RFMEM       = (1<<5), // share data and bss (kinda thread, a la Linux clone)
    RFNOWAIT    = (1<<6), // child will not leave a waitmsg

    RFNAMEG     = (1<<0), // copy namespace (if unset then share)
    RFENVG      = (1<<1), // copy environment variables (if unset then share)
    RFFDG       = (1<<2), // copy file descriptor table (if unset then share)

    RFCNAMEG    = (1<<10), // clean new namespace
    RFCENVG     = (1<<11), // clean new empty environment variables
    RFCFDG      = (1<<12), // clean new file descriptor table

    RFNOTEG     = (1<<3), // start new group for notes
    RFREND      = (1<<13), // start a new group for rendezvous
    RFNOMNT     = (1<<14), // # paths forbidden, sandboxing
};
/*e: enum rfork */

/*s: sysproc.c forward decl */
int shargs(char*, int, char**);
/*e: sysproc.c forward decl */

/*s: syscall nop */
// void nop(void);
long
sysnop(ulong*)
{
    print("Hello World\n");
    return 0;
}
/*e: syscall nop */

/*s: syscall rfork */
// int rfork(int flags);
long
sysrfork(ulong* arg)
{
    Proc *p;
    int i;
    bool share;
    Fgrp *ofg;
    Pgrp *opg;
    Rgrp *org;
    Egrp *oeg;
    ulong pid, flag;
    Cpu *wm;

    flag = arg[0];
    /* Check flags before we commit */
    if((flag & (RFFDG|RFCFDG)) == (RFFDG|RFCFDG))
        error(Ebadarg);
    if((flag & (RFNAMEG|RFCNAMEG)) == (RFNAMEG|RFCNAMEG))
        error(Ebadarg);
    if((flag & (RFENVG|RFCENVG)) == (RFENVG|RFCENVG))
        error(Ebadarg);

    if((flag&RFPROC) == 0) { // not a fork, just setting properties for up
        if(flag & (RFMEM|RFNOWAIT))
            error(Ebadarg);
        if(flag & (RFFDG|RFCFDG)) {
            ofg = up->fgrp;
            if(flag & RFFDG)
                up->fgrp = dupfgrp(ofg);
            else
                up->fgrp = dupfgrp(nil);
            closefgrp(ofg);
        }
        if(flag & (RFNAMEG|RFCNAMEG)) {
            opg = up->pgrp;
            up->pgrp = newpgrp();
            if(flag & RFNAMEG)
                pgrpcpy(up->pgrp, opg);
            /*s: [[sysrfork()]] inherit noattach, RFPROC==0 case */
            /* inherit noattach */
            up->pgrp->noattach = opg->noattach;
            /*e: [[sysrfork()]] inherit noattach, RFPROC==0 case */
            closepgrp(opg);
        }
       /*s: [[sysrfork()]] set noattach to true when RFNOMNT, RFPROC==0 case */
       if(flag & RFNOMNT)
           up->pgrp->noattach = true;
       /*e: [[sysrfork()]] set noattach to true when RFNOMNT, RFPROC==0 case */
        if(flag & RFREND) {
            org = up->rgrp;
            up->rgrp = newrgrp();
            closergrp(org);
        }
        if(flag & (RFENVG|RFCENVG)) {
            oeg = up->egrp;
            up->egrp = smalloc(sizeof(Egrp)); // newegrp()
            up->egrp->ref = 1;
            if(flag & RFENVG)
                envcpy(up->egrp, oeg);
            closeegrp(oeg);
        }
        if(flag & RFNOTEG)
            up->noteid = incref(&noteidalloc);
        return 0;
    }
    // ok RFPROC is set, let's create a new process

    p = newproc();
    pid = p->pid;

    p->sargs = up->sargs;
    p->slash = up->slash;
    p->dot = up->dot;
    incref(p->dot);
    memmove(p->note, up->note, sizeof(p->note));
    p->privatemem = up->privatemem;
    p->noswap = up->noswap;
    p->nnote = up->nnote;
    p->lastnote = up->lastnote;
    p->notify = up->notify;
    p->ureg = up->ureg;
    /*s: [[sysrfork()]] propagate fpsave */
        p->fpsave = up->fpsave;
    /*e: [[sysrfork()]] propagate fpsave */

    /* Make a new set of memory segments */
    share = flag & RFMEM;
    qlock(&p->seglock);
    if(waserror()){
        qunlock(&p->seglock);
        nexterror();
    }
    for(i = 0; i < NSEG; i++)
        if(up->seg[i])
            p->seg[i] = dupseg(up->seg, i, share);
    qunlock(&p->seglock);
    poperror();

    /* File descriptors */
    if(flag & (RFFDG|RFCFDG)) {
        if(flag & RFFDG)
            p->fgrp = dupfgrp(up->fgrp);
        else
            p->fgrp = dupfgrp(nil);
    }
    else {
        p->fgrp = up->fgrp;
        incref(p->fgrp);
    }

    /* Process groups */
    if(flag & (RFNAMEG|RFCNAMEG)) {
        p->pgrp = newpgrp();
        if(flag & RFNAMEG)
            pgrpcpy(p->pgrp, up->pgrp);
        /*s: [[sysrfork()]] inherit noattach, RFPROC==1 case */
        /* inherit noattach */
        p->pgrp->noattach = up->pgrp->noattach;
        /*e: [[sysrfork()]] inherit noattach, RFPROC==1 case */
    }
    else {
        p->pgrp = up->pgrp;
        incref(p->pgrp);
    }
    /*s: [[sysrfork()]] set noattach to true when RFNOMNT, RFPROC==1 case */
    if(flag & RFNOMNT)
        p->pgrp->noattach = true;
    /*e: [[sysrfork()]] set noattach to true when RFNOMNT, RFPROC==1 case */

    if(flag & RFREND)
        p->rgrp = newrgrp();
    else {
        incref(up->rgrp);
        p->rgrp = up->rgrp;
    }

    /* Environment group */
    if(flag & (RFENVG|RFCENVG)) {
        p->egrp = smalloc(sizeof(Egrp)); // newegrp
        p->egrp->ref = 1;
        if(flag & RFENVG)
            envcpy(p->egrp, up->egrp);
    }
    else {
        p->egrp = up->egrp;
        incref(p->egrp);
    }

    /*s: [[sysrfork()]] inherit hang */
        p->hang = up->hang;
    /*e: [[sysrfork()]] inherit hang */
    p->procmode = up->procmode;

    /* Craft a return frame which will cause the child to pop out of
     * the scheduler in user mode with the return register zero
     */
    arch_forkchild(p, up->dbgreg);

    p->parent = up;
    p->parentpid = up->pid;
    if(flag&RFNOWAIT)
        p->parentpid = 0;
    else {
        lock(&up->exl);
        up->nchild++;
        unlock(&up->exl);
    }
    if((flag&RFNOTEG) == 0)
        p->noteid = up->noteid;

    /*s: [[sysrfork()]] propagate fpstate */
        /* don't penalize the child, it hasn't done FP in a note handler. */
        p->fpstate = up->fpstate & ~FPillegal;
    /*e: [[sysrfork()]] propagate fpstate */
    /*s: [[sysrfork()]] setting time field */
    memset(p->time, 0, sizeof(p->time));
    p->time[TReal] = CPUS(0)->ticks;
    /*e: [[sysrfork()]] setting time field */

    kstrdup(&p->text, up->text);
    kstrdup(&p->user, up->user);

    /*
     *  since the bss/data segments are now shareable,
     *  any mmu info about this process is now stale
     *  (i.e. has bad properties) and has to be discarded.
     */
    arch_flushmmu();

    p->basepri = up->basepri;
    p->priority = up->basepri;
    p->fixedpri = up->fixedpri;
    p->lastcpu = up->lastcpu;
    wm = up->wired;
    if(wm)
        procwired(p, wm->cpuno);

    ready(p);
    sched();
    return pid;
}
/*e: syscall rfork */

/*s: function l2be */
ulong
l2be(long l)
{
    byte *cp;

    cp = (byte*)&l;
    return (cp[0]<<24) | (cp[1]<<16) | (cp[2]<<8) | cp[3];
}
/*e: function l2be */

/*s: syscall exec */
// void* exec(char *name, char* argv[]);
long
sysexec(ulong* arg)
{

    /*s: [[sysexec()]] locals */
    char *file;
    char *elem; // last element of binary, for up->text
    char *file0;
    /*x: [[sysexec()]] locals */
    Chan *tc;
    /*x: [[sysexec()]] locals */
    Exec exec;
    ulong magic, text, entry, data, bss;
    /*x: [[sysexec()]] locals */
    ulong t, d, b; // text, data, bss sizes in bytes rounded to pages
    Segment *s, *ts;
    /*x: [[sysexec()]] locals */
    char **argv, **argp;
    char *a, *charp, *args;
    ulong ssize, spage, nargs, nbytes;
    /*x: [[sysexec()]] locals */
    int i, n;
    /*x: [[sysexec()]] locals */
    KImage *img;
    /*x: [[sysexec()]] locals */
    Fgrp *f;
    /*x: [[sysexec()]] locals */
    Tos *tos;
    /*x: [[sysexec()]] locals */
    char line[sizeof(Exec)];
    char *progarg[sizeof(Exec)/2+1];
    char progelem[64];

    bool indir = false;
    /*e: [[sysexec()]] locals */

    elem = nil;
    validaddr(arg[0], 1, false);
    file0 = validnamedup((char*)arg[0], true);
    if(waserror()){
        free(file0);
        free(elem);
        nexterror();
    }
    file = file0;
    for(;;){
        // this will also adjust up->genbuf to contain the last element of file path
        /*s: [[sysexec()]] call namec() to get a channel in tc from file */
        tc = namec(file, Aopen, OEXEC, 0);
        /*e: [[sysexec()]] call namec() to get a channel in tc from file */
        if(waserror()){
            cclose(tc);
            nexterror();
        }
        if(!indir)
            kstrdup(&elem, up->genbuf);

        n = devtab[tc->type]->read(tc, &exec, sizeof(Exec), 0);
        if(n < 2) // need at least 2 bytes to decide if a #! or real binary
            error(Ebadexec);
        magic = l2be(exec.magic);
        text = l2be(exec.text);
        entry = l2be(exec.entry);

        if(n==sizeof(Exec) && (magic == AOUT_MAGIC)){
            if(text >= USTKTOP-UTZERO
            || entry < UTZERO+sizeof(Exec)
            || entry >= UTZERO+sizeof(Exec)+text)
                error(Ebadexec);
            break; /* for binary */
        }

        /*s: [[sysexec()]] process sharpbang */
        /*
         * Process #! /bin/sh args ...
         */
        memmove(line, &exec, sizeof(Exec));
        if(indir || line[0]!='#' || line[1]!='!')
            error(Ebadexec);
        n = shargs(line, n, progarg);
        if(n == 0)
            error(Ebadexec);
        indir = true;
        /*
         * First arg becomes complete file name
         */
        progarg[n++] = file;
        progarg[n] = 0;
        validaddr(arg[1], BY2WD, true);
        arg[1] += BY2WD;
        file = progarg[0];
        if(strlen(elem) >= sizeof progelem)
            error(Ebadexec);
        strcpy(progelem, elem);
        progarg[0] = progelem;
        poperror();
        cclose(tc);
        /*e: [[sysexec()]] process sharpbang */
    }

    data = l2be(exec.data);
    bss = l2be(exec.bss);
    t = UTROUND(UTZERO+sizeof(Exec)+text);
    // data is put at page boundary after text (see also _multibootentry)
    d = ROUND(t + data, BY2PG);
    // note that not t + d + bss but t + data + bss here
    b = ROUND(t + data + bss, BY2PG);
    if(t >= KZERO || d >= KZERO || b >= KZERO)
        error(Ebadexec);

    /*
     * Args: pass 1: count
     */
    nargs = 0;
    nbytes = 0;

    /*s: [[sysexec()]] nbytes tos adjustments */
    nbytes += sizeof(Tos); /* hole for profiling clock at top of stack (and more) */
    /*e: [[sysexec()]] nbytes tos adjustments */
    /*s: [[sysexec()]] if indir arg adjustments */
    if(indir){
        argp = progarg;
        while(*argp){
            a = *argp++;
            nbytes += strlen(a) + 1;
            nargs++;
        }
    }
    /*e: [[sysexec()]] if indir arg adjustments */

    arch_validalign(arg[1], sizeof(char**));
    argp = (char**)arg[1];
    validaddr((ulong)argp, BY2WD, false);
    while(*argp){
        a = *argp++;
        if(((ulong)argp&(BY2PG-1)) < BY2WD)
            validaddr((ulong)argp, BY2WD, false);
        validaddr((ulong)a, 1, false);
        nbytes += ((char*)vmemchr(a, 0, 0x7FFFFFFF) - a) + 1;
        nargs++;
    }
    ssize = BY2WD*(nargs+1) + ROUND(nbytes, BY2WD);

    /*
     * 8-byte align SP for those (e.g. sparc) that need it.
     * arch_execregs() will subtract another 4 bytes for argc.
     */
    if((ssize+4) & 7)
        ssize += 4;
    spage = (ssize+(BY2PG-1)) >> PGSHIFT;

    /*
     * Build the stack segment, putting it in kernel virtual for the moment
     */
    if(spage > TSTKSIZ)
        error(Enovmem);

    qlock(&up->seglock);
    if(waserror()){
        qunlock(&up->seglock);
        nexterror();
    }
    // why ESEG? and why not TSTKTOP?? ESEG because will free later
    // the current SSEG. If we have an error in sysexec we don't want
    // to have messed up with the current stack. TSTKTOP-USTKSIZE for
    // the same reason, because we don't want overwrite old stack
    // all of that will be relocated later.
    up->seg[ESEG] = newseg(SG_STACK, TSTKTOP-USTKSIZE, USTKSIZE/BY2PG);

    /*
     * Args: pass 2: assemble; the pages will be faulted in
     */
    /*s: [[sysexec()]] tos settings */
        tos = (Tos*)(TSTKTOP - sizeof(Tos));

        tos->cyclefreq = cpu->cyclefreq;
        cycles((uvlong*)&tos->pcycles);
        tos->pcycles = -tos->pcycles; // see comment above on Proc->pcycle
        tos->kcycles = tos->pcycles;
        tos->clock = 0;
        // what about other fields? like pid? will be set in kexit! but could be
        // done here? what about sysrfork? call kexit?
    /*e: [[sysexec()]] tos settings */

    argv = (char**)(TSTKTOP - ssize);
    charp = (char*)(TSTKTOP - nbytes);
    args = charp;
    /*s: [[sysexec()]] if indir argp adjustments */
    if(indir)
        argp = progarg;
    /*e: [[sysexec()]] if indir argp adjustments */
    else
        argp = (char**)arg[1];

    for(i=0; i<nargs; i++){
        /*s: [[sysexec()]] if indir argp adjustments again */
        if(indir && *argp==nil) {
            indir = false;
            argp = (char**)arg[1];
        }
        /*e: [[sysexec()]] if indir argp adjustments again */
        *argv++ = charp + (USTKTOP-TSTKTOP);
        n = strlen(*argp) + 1;
        memmove(charp, *argp++, n);
        charp += n;
    }
    free(file0);
    // file0 = nil? to avoid double free?

    free(up->text);
    up->text = elem;
    elem = nil; /* so waserror() won't free elem */
    USED(elem);

    /* copy args; easiest from new process's stack */
    n = charp - args;
    if(n > 128) /* don't waste too much space on huge arg lists */
        n = 128;
    a = up->args;
    up->args = nil;
    free(a);
    up->args = smalloc(n);
    memmove(up->args, args, n);

    if(n>0 && up->args[n-1]!='\0'){
        /* make sure last arg is NUL-terminated */
        /* put NUL at UTF-8 character boundary */
        for(i=n-1; i>0; --i)
            if(fullrune(up->args+i, n-i))
                break;
        up->args[i] = 0;
        n = i+1;
    }
    up->nargs = n;

    /*
     * Committed.
     * Free old memory.
     * Special segments are maintained across exec
     */
    for(i = SSEG; i <= BSEG; i++) {
        putseg(up->seg[i]);
        /* prevent a second free if we have an error */
        up->seg[i] = nil;
    }
    for(i = BSEG+1; i < NSEG; i++) {
        s = up->seg[i];
        if(s != nil && (s->type&SG_CEXEC)) { // close on exec
            putseg(s);
            up->seg[i] = nil;
        }
    }

    /*s: [[sysexec()]] close files marked as opened with close on exec */
    /*
     * Close on exec
     */
    f = up->fgrp;
    for(i=0; i<=f->maxfd; i++)
        fdclose(i, CCEXEC);
    /*e: [[sysexec()]] close files marked as opened with close on exec */

    /* Text.  Shared. Attaches to cache image if possible */
    /*s: [[sysexec()]] get text segment ts via demand loading on tc */
        /* attachimage returns a locked cache image */
        img = attachimage(SG_TEXT|SG_RONLY, tc, UTZERO, (t-UTZERO)>>PGSHIFT);
        ts = img->s;
        ts->flushme = true;
        ts->fstart = 0;
        ts->flen = sizeof(Exec)+text;
        unlock(img);
    /*e: [[sysexec()]] get text segment ts via demand loading on tc */
    up->seg[TSEG] = ts;

    /* Data. Shared. */
    s = newseg(SG_DATA, t, (d-t)>>PGSHIFT);
    up->seg[DSEG] = s;
    /*s: [[sysexec()]] adjust data segment s for demand loading on tc */
    /* Attached by hand */
    incref(img);
    s->image = img;
    s->fstart = ts->fstart+ts->flen;
    s->flen = data;
    // data is also in binary
    /*e: [[sysexec()]] adjust data segment s for demand loading on tc */

    /* BSS. Zero fill on demand */
    up->seg[BSEG] = newseg(SG_BSS, d, (b-d)>>PGSHIFT); // 0 fill! see fixfault

    /*
     * Move the stack
     */
    s = up->seg[ESEG];
    up->seg[ESEG] = nil;
    up->seg[SSEG] = s;
    qunlock(&up->seglock);
    poperror(); /* seglock */
    poperror(); /* elem */ // really? I think this matches more the cclose(tc)
    s->base = USTKTOP-USTKSIZE;
    s->top = USTKTOP;
    relocateseg(s, USTKTOP-TSTKTOP);

    /*
     *  '/' processes are higher priority (hack to make /ip more responsive).
     */
    if(devtab[tc->type]->dc == L'/')
        up->basepri = PriRoot;
    up->priority = up->basepri;
    poperror();
    cclose(tc); // tc has still a reference in the img

    /*
     *  At this point, the mmu contains info about the old address
     *  space and needs to be flushed
     */
    arch_flushmmu();

    qlock(&up->debug);
    up->nnote = 0;
    up->notify = nil;
    up->notified = false;
    up->privatemem = false;
    arch_procsetup(up);
    qunlock(&up->debug);

    /*s: [[sysexec()]] if hang */
        if(up->hang)
            up->procctl = Proc_stopme;
    /*e: [[sysexec()]] if hang */

    return arch_execregs(entry, ssize, nargs);
}
/*e: syscall exec */

/*s: function shargs */
int
shargs(char *s, int n, char **ap)
{
    int i;

    s += 2;
    n -= 2;     /* skip #! */
    for(i=0; s[i]!='\n'; i++)
        if(i == n-1)
            return 0;
    s[i] = 0;
    *ap = 0;
    i = 0;
    for(;;) {
        while(*s==' ' || *s=='\t')
            s++;
        if(*s == 0)
            break;
        i++;
        *ap++ = s;
        *ap = 0;
        while(*s && *s!=' ' && *s!='\t')
            s++;
        if(*s == 0)
            break;
        else
            *s++ = 0;
    }
    return i;
}
/*e: function shargs */

/*s: syscall sleep */
// int sleep(long millisecs);
long
syssleep(ulong* arg)
{
    int n;
    n = arg[0];
    if(n <= 0) {
        /*s: [[syssleep()]] optional [[edfyield()]] for real-time scheduling */
                if (up->edf && (up->edf->flags & Admitted))
                    edfyield();
                else
        /*e: [[syssleep()]] optional [[edfyield()]] for real-time scheduling */
        yield();
        return 0;
    }
    if(n < TK2MS(1))
        n = TK2MS(1);
    tsleep(&up->sleepr, returnfalse, 0, n);
    return 0;
}
/*e: syscall sleep */

/*s: syscall alarm */
// long alarm(unsigned long millisecs);
long
sysalarm(ulong* arg)
{
    return procalarm(arg[0]);
}
/*e: syscall alarm */

/*s: syscall exits */
// void exits(char *msg);
long
sysexits(ulong* arg)
{
    char *status;
    char *inval = "invalid exit string";
    char buf[ERRMAX];

    status = (char*)arg[0];
    if(status){
        if(waserror())
            status = inval;
        else{
            validaddr((ulong)status, 1, false);
            if(vmemchr(status, 0, ERRMAX) == 0){
                memmove(buf, status, ERRMAX);
                buf[ERRMAX-1] = 0;
                status = buf;
            }
            poperror();
        }

    }
    pexit(status, /*freemem*/true);
    panic("pexit: should never reach this point");
    return -1; // unreachable
}
/*e: syscall exits */

/*s: syscall await */
// int await(char *s, int n);
long
sysawait(ulong* arg)
{
    int i;
    int pid;
    Waitmsg w; // allocated in stack!
    ulong n;

    n = arg[1];
    validaddr(arg[0], n, true);
    pid = pwait(&w);
    if(pid < 0)
        return -1;
    i = snprint((char*)arg[0], n, "%d %lud %lud %lud %q",
        w.pid,
        /*s: [[sysawait()]] snprint time field arguments */
        w.time[TUser], w.time[TSys], w.time[TReal],
        /*e: [[sysawait()]] snprint time field arguments */
        w.msg);

    return i;
}
/*e: syscall await */

/*s: function werrstr */
//@Scheck: this is also defined in libc, so it's supposed to override it? TODO
void werrstr(char *fmt, ...)
{
    va_list va;

    if(up == nil)
        return;

    va_start(va, fmt);
    vseprint(up->syserrstr, up->syserrstr+ERRMAX, fmt, va);
    va_end(va);
}
/*e: function werrstr */

/*s: function generrstr */
static long
generrstr(char *buf, uint nbuf)
{
    char tmp[ERRMAX];

    if(nbuf == 0)
        error(Ebadarg);
    validaddr((ulong)buf, nbuf, true);
    if(nbuf > sizeof tmp)
        nbuf = sizeof tmp;
    memmove(tmp, buf, nbuf);

    /* make sure it's NUL-terminated */
    tmp[nbuf-1] = '\0';
    memmove(buf, up->syserrstr, nbuf);
    buf[nbuf-1] = '\0';
    memmove(up->syserrstr, tmp, nbuf);
    return 0;
}
/*e: function generrstr */

/*s: syscall errstr */
// int errstr(char *err, uint nerr);
long
syserrstr(ulong* arg)
{
    return generrstr((char*)arg[0], arg[1]);
}
/*e: syscall errstr */

/*s: syscall notify */
// int notify(void (*f)(void*, char*));
long
sysnotify(ulong* arg)
{
    if(arg[0] != 0)
        validaddr(arg[0], sizeof(ulong), false);
    up->notify = (int(*)(void*, char*))(arg[0]);
    return 0;
}
/*e: syscall notify */

/*s: syscall noted */
// int noted(int v);
long
sysnoted(ulong* arg)
{
    if(arg[0]!=NRSTR && !up->notified)
        error(Egreg);
    return 0;
}
/*e: syscall noted */

/*s: syscall segbrk */
// void* segbrk(void *saddr, void *addr);
long
syssegbrk(ulong* arg)
{
    int i;
    ulong addr;
    Segment *s;

    addr = arg[0];
    for(i = 0; i < NSEG; i++) {
        s = up->seg[i];
        if(s == nil || addr < s->base || addr >= s->top)
            continue;
        switch(s->type&SG_TYPE) {
        case SG_TEXT:
        case SG_DATA:
        case SG_STACK:
            error(Ebadarg);
        default:
            return ibrk(arg[1], i);
        }
    }

    error(Ebadarg);
    panic("syssegbrk: should not reach this point");
    return -1; // unreachable
}
/*e: syscall segbrk */

/*s: syscall segattach */
// void* segattach(int attr, char *class, void *va, ulong len);
long
syssegattach(ulong* arg)
{
    return segattach(up, arg[0], (char*)arg[1], arg[2], arg[3]);
}
/*e: syscall segattach */

/*s: syscall segdetach */
// int segdetach(void *addr);
long
syssegdetach(ulong* arg)
{
    int i;
    ulong addr;
    Segment *s;

    qlock(&up->seglock);
    if(waserror()){
        qunlock(&up->seglock);
        nexterror();
    }

    s = 0;
    addr = arg[0];
    for(i = 0; i < NSEG; i++)
        if(s = up->seg[i]) {
            qlock(&s->lk);
            if((addr >= s->base && addr < s->top) ||
               (s->top == s->base && addr == s->base))
                goto found;
            qunlock(&s->lk);
        }

    error(Ebadarg);

found:
    /*
     * Check we are not detaching the initial stack segment.
     */
    if(s == up->seg[SSEG]){
        qunlock(&s->lk);
        error(Ebadarg);
    }
    up->seg[i] = 0;
    qunlock(&s->lk);
    putseg(s);
    qunlock(&up->seglock);
    poperror();

    /* Ensure we flush any entries from the lost segment */
    arch_flushmmu();
    return 0;
}
/*e: syscall segdetach */

/*s: syscall segfree */
// int segfree(void *va, ulong len);
long
syssegfree(ulong* arg)
{
    Segment *s;
    ulong from, to;

    from = arg[0];
    s = seg(up, from, 1);
    if(s == nil)
        error(Ebadarg);
    to = (from + arg[1]) & ~(BY2PG-1);
    from = PGROUND(from);

    if(to > s->top) {
        qunlock(&s->lk);
        error(Ebadarg);
    }

    mfreeseg(s, from, (to - from) / BY2PG);
    qunlock(&s->lk);
    arch_flushmmu();

    return 0;
}
/*e: syscall segfree */

/*s: syscall brk */
// int brk(void*);
long
sysbrk(ulong* arg)
{
    return ibrk(arg[0], BSEG); // BSS, the heap size is changed
}
/*e: syscall brk */

/*s: syscall rendezvous */
// void* rendezvous(void* tag, void* value);
long
sysrendezvous(ulong* arg)
{
    uintptr tag, val;
    Proc *p, **l;

    tag = arg[0];
    l = &REND(up->rgrp, tag);
    up->rendval = ~(uintptr)0;

    lock(up->rgrp);
    for(p = *l; p; p = p->rendhash) {
        if(p->rendtag == tag) {
            *l = p->rendhash;
            val = p->rendval;
            p->rendval = arg[1];

            while(p->cpu != 0) // ????
                ;

            ready(p);
            unlock(up->rgrp);
            return val;
        }
        l = &p->rendhash;
    }

    /* Going to sleep here */
    up->rendtag = tag;
    up->rendval = arg[1];
    up->rendhash = *l;
    *l = up;
    up->state = Rendezvous;
    unlock(up->rgrp);

    sched();

    return up->rendval;
}
/*e: syscall rendezvous */

/*e: sysproc.c */
