/*s: db/trcrun.c */
/*
 * functions for running the debugged process
 */

#include "defs.h"
#include "fns.h"


/*s: global child */
int child;
/*e: global child */
/*s: global msgfd */
int msgfd = -1;
/*e: global msgfd */
/*s: global notefd */
int notefd = -1;
/*e: global notefd */
/*s: global pcspid */
int pcspid = -1;
/*e: global pcspid */


/*s: function setpcs */
void
setpcs(void)
{
    char buf[128];

    if(pid && pid != pcspid){
        if(msgfd >= 0){
            close(msgfd);
            msgfd = -1;
        }
        if(notefd >= 0){
            close(notefd);
            notefd = -1;
        }
        pcspid = -1;
        sprint(buf, "/proc/%d/ctl", pid);
        msgfd = open(buf, OWRITE);
        if(msgfd < 0)
            error("can't open control file");
        sprint(buf, "/proc/%d/note", pid);
        notefd = open(buf, ORDWR);
        if(notefd < 0)
            error("can't open note file");
        pcspid = pid;
    }
}
/*e: function setpcs */

/*s: function msgpcs */
void
msgpcs(char *msg)
{
    char err[ERRMAX];

    setpcs();
    if(write(msgfd, msg, strlen(msg)) < 0 && !ending){
        errstr(err, sizeof err);
        if(strcmp(err, "interrupted") != 0)
            endpcs();
        errors("can't write control file", err);
    }
}
/*e: function msgpcs */

/*s: function unloadnote */
/*
 * empty the note buffer and toss pending breakpoint notes
 */
void
unloadnote(void)
{
    char err[ERRMAX];

    setpcs();
    for(; nnote<NNOTE; nnote++){
        switch(read(notefd, note[nnote], sizeof note[nnote])){
        case -1:
            errstr(err, sizeof err);
            if(strcmp(err, "interrupted") != 0)
                endpcs();
            errors("can't read note file", err);
        case 0:
            return;
        }
        note[nnote][ERRMAX-1] = '\0';
        if(strncmp(note[nnote], "sys: breakpoint", 15) == 0)
            --nnote;
    }
}
/*e: function unloadnote */

/*s: function loadnote */
/*
 * reload the note buffer
 */
void
loadnote(void)
{
    int i;
    char err[ERRMAX];

    setpcs();
    for(i=0; i<nnote; i++){
        if(write(notefd, note[i], strlen(note[i])) < 0){
            errstr(err, sizeof err);
            if(strcmp(err, "interrupted") != 0)
                endpcs();
            errors("can't write note file", err);
        }
    }
    nnote = 0;
}
/*e: function loadnote */

/*s: function notes */
void
notes(void)
{
    int n;

    if(nnote == 0)
        return;
    dprint("notes:\n");
    for(n=0; n<nnote; n++)
        dprint("%d:\t%s\n", n, note[n]);
}
/*e: function notes */

/*s: function killpcs */
void
killpcs(void)
{
    msgpcs("kill");
}
/*e: function killpcs */

/*s: function grab */
void
grab(void)
{
    flush();
    msgpcs("stop");
    bpwait();
}
/*e: function grab */

/*s: function ungrab */
void
ungrab(void)
{
    msgpcs("start");
}
/*e: function ungrab */

/*s: function doexec */
void
doexec(void)
{
    char *argl[MAXARG];
    char args[LINSIZ];
    char *p;
    char **ap;
    char *thisarg;

    ap = argl;
    p = args;
    *ap++ = symfil;
    for (rdc(); lastc != EOR;) {
        thisarg = p;
        if (lastc == '<' || lastc == '>') {
            *p++ = lastc;
            rdc();
        }
        while (lastc != EOR && lastc != SPC && lastc != TB) {
            *p++ = lastc;
            readchar();
        }
        if (lastc == SPC || lastc == TB)
            rdc();
        *p++ = 0;
        if (*thisarg == '<') {
            close(0);
            if (open(&thisarg[1], OREAD) < 0) {
                print("%s: cannot open\n", &thisarg[1]);
                _exits(0);
            }
        }
        else if (*thisarg == '>') {
            close(1);
            if (create(&thisarg[1], OWRITE, 0666) < 0) {
                print("%s: cannot create\n", &thisarg[1]);
                _exits(0);
            }
        }
        else
            *ap++ = thisarg;
    }
    *ap = 0;
    exec(symfil, argl);
    perror(symfil);
}
/*e: function doexec */

/*s: global procname */
char	procname[100];
/*e: global procname */

/*s: function startpcs */
void
startpcs(void)
{
    if ((pid = fork()) == 0) {
        pid = getpid();
        msgpcs("hang");
        doexec();
        exits(0);
    }

    if (pid == -1)
        error("can't fork");
    child++;
    sprint(procname, "/proc/%d/mem", pid);
    corfil = procname;
    msgpcs("waitstop");
    bpwait();
    if (adrflg)
        rput(cormap, mach->pc, adrval);
    while (rdc() != EOR)
        ;
    reread();
}
/*e: function startpcs */

/*s: function runstep */
void
runstep(uvlong loc, int keepnote)
{
    int nfoll;
    uvlong foll[3];
    BKPT bkpt[3];
    int i;

    if(machdata->foll == 0){
        dprint("stepping unimplemented; assuming not a branch\n");
        nfoll = 1;
        foll[0] = loc+mach->pcquant;
    }else {
        nfoll = machdata->foll(cormap, loc, rget, foll);
        if (nfoll < 0)
            error("%r");
    }
    memset(bkpt, 0, sizeof bkpt);
    for(i=0; i<nfoll; i++){
        if(foll[i] == loc)
            error("can't single step: next instruction is dot");
        bkpt[i].loc = foll[i];
        bkput(&bkpt[i], 1);
    }
    runrun(keepnote);
    for(i=0; i<nfoll; i++)
        bkput(&bkpt[i], 0);
}
/*e: function runstep */

/*s: function bpwait */
void
bpwait(void)
{
    setcor();
    unloadnote();
}
/*e: function bpwait */

/*s: function runrun */
void
runrun(int keepnote)
{
    int on;

    on = nnote;
    unloadnote();
    if(on != nnote){
        notes();
        error("not running: new notes pending");
    }
    if(keepnote)
        loadnote();
    else
        nnote = 0;
    flush();
    msgpcs("startstop");
    bpwait();
}
/*e: function runrun */

/*s: function bkput */
void
bkput(BKPT *bp, int install)
{
    char buf[256];
    ADDR loc;
    int ret;

    errstr(buf, sizeof buf);
    if(machdata->bpfix)
        loc = (*machdata->bpfix)(bp->loc);
    else
        loc = bp->loc;
    if(install){
        ret = get1(cormap, loc, bp->save, machdata->bpsize);
        if (ret > 0)
            ret = put1(cormap, loc, machdata->bpinst, machdata->bpsize);
    }else
        ret = put1(cormap, loc, bp->save, machdata->bpsize);
    if(ret < 0){
        sprint(buf, "can't set breakpoint at %#llux: %r", bp->loc);
        print(buf);
        read(0, buf, 100);
    }
}
/*e: function bkput */
/*e: db/trcrun.c */
