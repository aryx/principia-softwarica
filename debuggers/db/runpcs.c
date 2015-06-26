/*s: db/runpcs.c */
/*
 *
 *	debugger
 *
 */

#include "defs.h"
#include "fns.h"

/*s: global bpin */
bool bpin;
/*e: global bpin */

/*s: function runpcs */
/* service routines for sub process control */
int
runpcs(int runmode, bool keepnote)
{
    int rc = 0; // runcount
    BKPT *bkpt;

    if (adrflg)
        rput(cormap, mach->pc, dot);
    dot = rget(cormap, mach->pc);
    flush();

    while (loopcnt-- > 0) {
        if(loopcnt != 0)
            printpc();
        if (runmode == SINGLE) {
            /*s: [[runpcs()]] in SINGLE mode, clean breakpoint if at dot */
            bkpt = scanbkpt(dot);
            if (bkpt) {
                switch(bkpt->flag){
                case BKPTTMP:
                    bkpt->flag = BKPTCLR;
                    break;
                case BKPTSKIP:
                    bkpt->flag = BKPTSET;
                    break;
                }
            }
            /*e: [[runpcs()]] in SINGLE mode, clean breakpoint if at dot */
            runstep(dot, keepnote);
        } else {
            if ((bkpt = scanbkpt(rget(cormap, mach->pc))) != nil) {
                execbkpt(bkpt, keepnote);
                keepnote = false;
            }
            setbp();
            runrun(keepnote);
        }
        keepnote = false;
        delbp();
        dot = rget(cormap, mach->pc);

        /* real note? */
        if (nnote > 0) {
            keepnote = true;
            rc = 0;
            continue;
        }
        bkpt = scanbkpt(dot);
        if(bkpt == nil){
            keepnote = false;
            rc = 0;
            continue;
        }
        /* breakpoint */
        if (bkpt->flag == BKPTTMP)
            bkpt->flag = BKPTCLR;
        else if (bkpt->flag == BKPTSKIP) {
            execbkpt(bkpt, keepnote);
            keepnote = false;
            loopcnt++;	/* we didn't really stop */
            continue;
        }
        else {
            bkpt->flag = BKPTSKIP;
            --bkpt->count;
            if ((bkpt->comm[0] == EOR || command(bkpt->comm, ':') != 0)
                && bkpt->count != 0) {
                execbkpt(bkpt, keepnote);
                keepnote = false;
                loopcnt++;
                continue;
            }
            bkpt->count = bkpt->initcnt;
        }
        rc = 1;
    }
    return rc;
}
/*e: function runpcs */

/*s: function endpcs */
/*
 * finish the process off;
 * kill if still running
 */
void
endpcs(void)
{
    BKPT *bk;

    if(ending)
        return;
    ending = true;
    if (pid) {
        if(pcsactive){
            killpcs();
            pcsactive = false;
        }
        pid=0;
        nnote=0;
        for (bk=bkpthead; bk; bk = bk->nxtbkpt)
            if (bk->flag == BKPTTMP)
                bk->flag = BKPTCLR;
            else if (bk->flag != BKPTCLR)
                bk->flag = BKPTSET;
    }
    bpin = FALSE;
    ending = false;
}
/*e: function endpcs */

/*s: function setup */
/*
 * start up the program to be debugged in a child
 */
void
setup(void)
{

    nnote = 0;
    startpcs();
    pcsactive = true;
    bpin = FALSE;
}
/*e: function setup */

/*s: function execbkpt */
/*
 * skip over a breakpoint:
 * remove breakpoints, then single step
 * so we can put it back
 */
void
execbkpt(BKPT *bk, int keepnote)
{
    runstep(bk->loc, keepnote);
    bk->flag = BKPTSET;
}
/*e: function execbkpt */

/*s: function scanbkpt */
/*
 * find the breakpoint at adr, if any
 */
BKPT*
scanbkpt(ADDR adr)
{
    BKPT *bk;

    for (bk = bkpthead; bk; bk = bk->nxtbkpt)
        if (bk->flag != BKPTCLR && bk->loc == adr)
            break;
    return bk;
}
/*e: function scanbkpt */

/*s: function delbp */
/*
 * remove all breakpoints from the process' address space
 */

void
delbp(void)
{
    BKPT *bk;

    if (bpin == FALSE || pid == 0)
        return;
    for (bk = bkpthead; bk; bk = bk->nxtbkpt)
        if (bk->flag != BKPTCLR)
            bkput(bk, false);
    bpin = FALSE;
}
/*e: function delbp */

/*s: function setbp */
/*
 * install all the breakpoints
 */

void
setbp(void)
{
    BKPT *bk;

    if (bpin == TRUE || pid == 0)
        return;
    for (bk = bkpthead; bk; bk = bk->nxtbkpt)
        if (bk->flag != BKPTCLR)
            bkput(bk, true);
    bpin = TRUE;
}
/*e: function setbp */
/*e: db/runpcs.c */
