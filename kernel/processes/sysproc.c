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

/*s: enum [[rfork]] */
//coupling: with libc.h
enum Rfork
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
/*e: enum [[rfork]] */

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
    ulong flag;
    /*s: [[sysrfork()]] other locals */
    Proc *p;
    ulong pid;
    /*x: [[sysrfork()]] other locals */
    bool share;
    int i;
    /*x: [[sysrfork()]] other locals */
    Fgrp *ofg;
    Pgrp *opg;
    Egrp *oeg;
    Rgrp *org;
    /*x: [[sysrfork()]] other locals */
    Cpu *wm;
    /*e: [[sysrfork()]] other locals */

    flag = arg[0];
    /* Check flags before we commit */
    /*s: [[sysrfork()]] sanity check flags, clean or copy */
    if((flag & (RFFDG|RFCFDG)) == (RFFDG|RFCFDG))
        error(Ebadarg);
    /*x: [[sysrfork()]] sanity check flags, clean or copy */
    if((flag & (RFNAMEG|RFCNAMEG)) == (RFNAMEG|RFCNAMEG))
        error(Ebadarg);
    /*x: [[sysrfork()]] sanity check flags, clean or copy */
    if((flag & (RFENVG|RFCENVG)) == (RFENVG|RFCENVG))
        error(Ebadarg);
    /*e: [[sysrfork()]] sanity check flags, clean or copy */

    /*s: [[sysrfork()]] if no [[RFPROC]], no new proc but property settings */
    if((flag&RFPROC) == 0) { // not a fork, just setting properties for up
        /*s: [[sysrfork()]] when no [[RFPROC]], sanity check flag */
        if(flag & (RFMEM|RFNOWAIT))
            error(Ebadarg);
        /*e: [[sysrfork()]] when no [[RFPROC]], sanity check flag */

        /*s: [[sysrfork()]] when no [[RFPROC]], clean or copy file descriptors */
        if(flag & (RFFDG|RFCFDG)) {
            ofg = up->fgrp;
            if(flag & RFFDG)
                up->fgrp = dupfgrp(ofg);
            else
                up->fgrp = dupfgrp(nil);
            closefgrp(ofg);
        }
        /*e: [[sysrfork()]] when no [[RFPROC]], clean or copy file descriptors */

        /*s: [[sysrfork()]] when no [[RFPROC]], clean or copy namespace */
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
        /*e: [[sysrfork()]] when no [[RFPROC]], clean or copy namespace */

        /*s: [[sysrfork()]] when no [[RFPROC]], clean or copy environment */
        if(flag & (RFENVG|RFCENVG)) {
            oeg = up->egrp;
            up->egrp = smalloc(sizeof(Egrp)); // newegrp()
            up->egrp->ref = 1;
            if(flag & RFENVG)
                envcpy(up->egrp, oeg);
            closeegrp(oeg);
        }
        /*e: [[sysrfork()]] when no [[RFPROC]], clean or copy environment */

        /*s: [[sysrfork()]] when no [[RFPROC]], new rendez vous group */
        if(flag & RFREND) {
            org = up->rgrp;
            up->rgrp = newrgrp();
            closergrp(org);
        }
        /*e: [[sysrfork()]] when no [[RFPROC]], new rendez vous group */

        /*s: [[sysrfork()]] when no [[RFPROC]], new note group */
        if(flag & RFNOTEG)
            up->noteid = incref(&noteidalloc);
        /*e: [[sysrfork()]] when no [[RFPROC]], new note group */

        return 0;
    }
    /*e: [[sysrfork()]] if no [[RFPROC]], no new proc but property settings */
    // else
    /*s: [[sysrfork()]] when [[RFPROC]], fork a new process */
    p = newproc();
    pid = p->pid;

    /*s: [[sysrfork()]] when [[RFPROC]] propagate fields from up to p */
    kstrdup(&p->text, up->text);
    kstrdup(&p->user, up->user);
    /*x: [[sysrfork()]] when [[RFPROC]] propagate fields from up to p */
    p->slash = up->slash;
    p->dot = up->dot;
    incref(p->dot);
    /*x: [[sysrfork()]] when [[RFPROC]] propagate fields from up to p */
    p->sargs = up->sargs;
    /*x: [[sysrfork()]] when [[RFPROC]] propagate fields from up to p */
    p->noswap = up->noswap;
    /*x: [[sysrfork()]] when [[RFPROC]] propagate fields from up to p */
    p->priority = up->basepri;
    p->basepri = up->basepri;
    p->fixedpri = up->fixedpri;
    /*x: [[sysrfork()]] when [[RFPROC]] propagate fields from up to p */
    p->lastcpu = up->lastcpu;
    /*x: [[sysrfork()]] when [[RFPROC]] propagate fields from up to p */
    memmove(p->note, up->note, sizeof(p->note));
    p->nnote = up->nnote;
    p->lastnote = up->lastnote;
    p->notify = up->notify;
    /*x: [[sysrfork()]] when [[RFPROC]] propagate fields from up to p */
    p->ureg = up->ureg; // will be adjusted for R0??
    /*x: [[sysrfork()]] when [[RFPROC]] propagate fields from up to p */
    p->hang = up->hang;
    /*x: [[sysrfork()]] when [[RFPROC]] propagate fields from up to p */
    /*s: [[sysrfork()]] propagate fpstate */
    /* don't penalize the child, it hasn't done FP in a note handler. */
    p->fpstate = up->fpstate & ~FPillegal;
    /*e: [[sysrfork()]] propagate fpstate */
    /*x: [[sysrfork()]] when [[RFPROC]] propagate fields from up to p */
    /*s: [[sysrfork()]] propagate fpsave */
    p->fpsave = up->fpsave;
    /*e: [[sysrfork()]] propagate fpsave */
    /*x: [[sysrfork()]] when [[RFPROC]] propagate fields from up to p */
    p->procmode = up->procmode;
    /*x: [[sysrfork()]] when [[RFPROC]] propagate fields from up to p */
    p->privatemem = up->privatemem;
    /*e: [[sysrfork()]] when [[RFPROC]] propagate fields from up to p */

    /* Make a new set of memory segments */
    /*s: [[sysrfork()]] copy or share memory segments */
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
    /*e: [[sysrfork()]] copy or share memory segments */

    /* File descriptors */
    /*s: [[sysrfork()]] copy, clean, or share file descriptors */
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
    /*e: [[sysrfork()]] copy, clean, or share file descriptors */
    /* Process groups */ // Namespace
    /*s: [[sysrfork()]] copy, clean, or share namespace */
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
    /*e: [[sysrfork()]] copy, clean, or share namespace */
    // Rendez vous group
    /*s: [[sysrfork()]] new or share rendezvous group */
    if(flag & RFREND)
        p->rgrp = newrgrp();
    else {
        incref(up->rgrp);
        p->rgrp = up->rgrp;
    }
    /*e: [[sysrfork()]] new or share rendezvous group */
    /* Environment group */
    /*s: [[sysrfork()]] copy, clean, or share environment */
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
    /*e: [[sysrfork()]] copy, clean, or share environment */
    // Note group
    /*s: [[sysrfork()]] share or not note group */
    if((flag&RFNOTEG) == 0)
        p->noteid = up->noteid; // rectify what was done in newproc
    /*e: [[sysrfork()]] share or not note group */

    /* Craft a return frame which will cause the child to pop out of
     * the scheduler in user mode with the return register zero
     */
    arch_forkchild(p, up->dbgreg);

    p->parent = up;
    p->parentpid = up->pid;
    /*s: [[sysrfork()]] wait or not for child */
    if(flag&RFNOWAIT)
        p->parentpid = 0;
    else {
        lock(&up->exl);
        up->nchild++;
        unlock(&up->exl);
    }
    /*e: [[sysrfork()]] wait or not for child */

    /*s: [[sysrfork()]] setting time field */
    memset(p->time, 0, sizeof(p->time));
    p->time[TReal] = CPUS(0)->ticks;
    /*e: [[sysrfork()]] setting time field */

    /*
     *  since the bss/data segments are now shareable,
     *  any mmu info about this process is now stale
     *  (i.e. has bad properties) and has to be discarded.
     */
    arch_flushmmu();

    /*s: [[sysrfork()]] if parent is a wired proc */
    wm = up->wired;
    if(wm)
        procwired(p, wm->cpuno);
    /*e: [[sysrfork()]] if parent is a wired proc */

    ready(p);
    sched(); // !!!
    return pid;
    /*e: [[sysrfork()]] when [[RFPROC]], fork a new process */
}
/*e: syscall rfork */

/*s: function [[l2be]] */
ulong
l2be(long l)
{
    byte *cp;

    cp = (byte*)&l;
    return (cp[0]<<24) | (cp[1]<<16) | (cp[2]<<8) | cp[3];
}
/*e: function [[l2be]] */

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
    ulong magic;
    user_addr text, entry, data, bss;
    /*x: [[sysexec()]] locals */
    user_addr t, d, b; // end of text/data/bss in address rounded to page
    Segment *s, *ts;
    /*x: [[sysexec()]] locals */
    ulong ssize;
    ulong nargs;
    /*x: [[sysexec()]] locals */
    int i, n;
    /*x: [[sysexec()]] locals */
    char **argp;
    char *a, *args;
    ulong nbytes;
    /*x: [[sysexec()]] locals */
    ulong spage;
    /*x: [[sysexec()]] locals */
    char **argv;
    char *charp;
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
        // this will also adjust up->genbuf to contain the last elt of file path
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

            // else
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
    // from break above
    data = l2be(exec.data);
    bss = l2be(exec.bss);

    /*s: [[sysexec()]] compute end of text, data, bss segments */
    t = UTROUND(UTZERO+sizeof(Exec)+text);
    // data is put at page boundary after text
    d = ROUND(t + data, BY2PG);
    // note that this is not d + bss here, but t + data + bss!
    b = ROUND(t + data + bss, BY2PG);

    if(t >= KZERO || d >= KZERO || b >= KZERO)
        error(Ebadexec);
    /*e: [[sysexec()]] compute end of text, data, bss segments */


    /*s: [[sysexec()]] build args count */
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
    /*e: [[sysexec()]] build args count */

    /*s: [[sysexec()]] build temporary stack segment and qlock seglock */
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
    /*e: [[sysexec()]] build temporary stack segment and qlock seglock */

    /*s: [[sysexec()]] build args */
    /*
     * Args: pass 2: assemble; the pages will be faulted in
     */
    /*s: [[sysexec()]] tos settings */
    tos = (Tos*)(TSTKTOP - sizeof(Tos));

    tos->cyclefreq = cpu->cyclefreq;
    arch_cycles((uvlong*)&tos->pcycles);
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
    /*e: [[sysexec()]] build args */

    /*s: [[sysexec()]] free old memory segments */
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
    /*s: [[sysexec()]] free special segments if close on exec */
    for(i = BSEG+1; i < NSEG; i++) {
        s = up->seg[i];
        if(s != nil && (s->type&SG_CEXEC)) { // close on exec
            putseg(s);
            up->seg[i] = nil;
        }
    }
    /*e: [[sysexec()]] free special segments if close on exec */
    /*e: [[sysexec()]] free old memory segments */

    /*s: [[sysexec()]] close files marked as opened with close on exec */
    /*
     * Close on exec
     */
    f = up->fgrp;
    for(i=0; i<=f->maxfd; i++)
        fdclose(i, CCEXEC);
    /*e: [[sysexec()]] close files marked as opened with close on exec */


    /*s: [[sysexec()]] set Text segment */
    /* Text.  Shared. Attaches to cache image if possible */
    /*s: [[sysexec()]] get text segment ts via demand loading on tc */
    /* attachimage returns a locked cache image */
    img = attachimage(SG_TEXT|SG_RONLY, tc, UTZERO, (t-UTZERO)>>PGSHIFT);
    ts = img->s;
    ts->fstart = 0;
    ts->flen = sizeof(Exec)+text;
    /*s: [[sysexec()]] initialize other fields */
    ts->flushme = true;
    /*e: [[sysexec()]] initialize other fields */
    unlock(img);
    /*e: [[sysexec()]] get text segment ts via demand loading on tc */
    up->seg[TSEG] = ts;
    /*e: [[sysexec()]] set Text segment */

    /*s: [[sysexec()]] set Data segment */
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
    /*e: [[sysexec()]] set Data segment */

    /*s: [[sysexec()]] set Bss segment */
    /* BSS. Zero fill on demand */
    up->seg[BSEG] = newseg(SG_BSS, d, (b-d)>>PGSHIFT);
    /*e: [[sysexec()]] set Bss segment */

    /*s: [[sysexec()]] relocate temporary stack segment, unlock seglock */
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
    /*e: [[sysexec()]] relocate temporary stack segment, unlock seglock */

    /*s: [[sysexec()]] set prioriry of root processes */
    /*
     *  '/' processes are higher priority (hack to make /ip more responsive).
     */
    if(devtab[tc->type]->dc == L'/')
        up->basepri = PriRoot;
    up->priority = up->basepri;
    /*e: [[sysexec()]] set prioriry of root processes */

    poperror();
    cclose(tc); // tc has still a reference in the img

    /*
     *  At this point, the mmu contains info about the old address
     *  space and needs to be flushed
     */
    arch_flushmmu();

    qlock(&up->debug);
    /*s: [[sysexec()]] when hold debug lock, reset some fields */
    up->nnote = 0;
    up->notify = nil;
    up->notified = false;
    /*x: [[sysexec()]] when hold debug lock, reset some fields */
    up->privatemem = false;
    /*e: [[sysexec()]] when hold debug lock, reset some fields */
    arch_procsetup(up);
    qunlock(&up->debug);

    /*s: [[sysexec()]] if hang */
    if(up->hang)
        up->procctl = Proc_stopme;
    /*e: [[sysexec()]] if hang */

    return arch_execregs(entry, ssize, nargs);
}
/*e: syscall exec */

/*s: function [[shargs]] */
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
/*e: function [[shargs]] */

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
    /*s: [[syssleep()]] sanitize n */
    if(n < TK2MS(1))
        n = TK2MS(1);
    /*e: [[syssleep()]] sanitize n */
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
    /*s: [[sysexits()]] other locals */
    char *inval = "invalid exit string";
    char buf[ERRMAX];
    /*e: [[sysexits()]] other locals */

    status = (char*)arg[0];
    /*s: [[sysexits()]] sanity check status, copy string in buf */
    if(status){
        if(waserror())
            status = inval;
        else{
            // validaddr() and vmemchr() can generate error()
            validaddr((ulong)status, 1, false);
            if(vmemchr(status, '\0', ERRMAX) == 0){
                memmove(buf, status, ERRMAX);
                buf[ERRMAX-1] = '\0';
                status = buf;
            }
            poperror();
        }

    }
    /*e: [[sysexits()]] sanity check status, copy string in buf */
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
    /*s: [[sysawait()]] sanity check pid */
    if(pid < 0)
        return -1;
    /*e: [[sysawait()]] sanity check pid */
    i = snprint((char*)arg[0], n, "%d %lud %lud %lud %q",
        w.pid,
        /*s: [[sysawait()]] snprint time field arguments */
        w.time[TUser], w.time[TSys], w.time[TReal],
        /*e: [[sysawait()]] snprint time field arguments */
        w.msg);

    return i;
}
/*e: syscall await */

/*s: function [[werrstr]] */
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
/*e: function [[werrstr]] */

/*s: function [[generrstr]] */
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
/*e: function [[generrstr]] */

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
