/*s: db/pcs.c */
/*
 *
 *	debugger
 *
 */

#include "defs.h"
#include "fns.h"

/*s: global [[NOPCS]] */
char	NOPCS[] = "no process";
/*e: global [[NOPCS]] */

/*s: function [[subpcs]] */
/* sub process control */

void
subpcs(int modif)
{
    // enum<runmode>
    int	runmode = SINGLE;
    bool keepnote = false;
    int	check;
    int	n;
    int r = 0;
    long line, curr;
    BKPT *bk;
    char *comptr;

    loopcnt=cntval;

    switch (modif) {
    /*s: [[subpcs()]] switch modif cases */
    /* run program */
    case 'r': 
    case 'R':
        endpcs();
        setup();
        runmode = CONTIN;
        break;
    /*x: [[subpcs()]] switch modif cases */
    /* single step */
    case 's': 
        if (pid == 0) {
            setup();
            loopcnt--;
        }
        runmode=SINGLE;
        keepnote=defval(true);
        break;
    /*x: [[subpcs()]] switch modif cases */
    /* exit */
    case 'k' :
    case 'K':
        if (pid == 0)
            error(NOPCS);
        dprint("%d: killed", pid);
        pcsactive = true;	/* force 'kill' ctl */
        endpcs();
        return;
    /*x: [[subpcs()]] switch modif cases */
    /* halt the current process */
    case 'h':	
        /*s: [[subpcs()]] halting case, if addr 0 specified */
        if (adrflg && adrval == 0) {
            if (pid == 0)
                error(NOPCS);
            ungrab();
        }
        /*e: [[subpcs()]] halting case, if addr 0 specified */
        else {
            grab();
            dprint("stopped at%16t");
            goto Return;
        }
        return;
    /*x: [[subpcs()]] switch modif cases */
    /* continue executing the current process */
    case 'x':	
        if (pid == 0)
            error(NOPCS);
        ungrab();
        return;
    /*x: [[subpcs()]] switch modif cases */
    /* set breakpoint */
    case 'b': 
    case 'B':
        if (bk=scanbkpt(dot))
            bk->flag=BKPTCLR;

        /*s: [[subpcs()]] breakpoint case, find unused breakpoint bk or allocate one */
        for (bk=bkpthead; bk; bk=bk->nxtbkpt)
            if (bk->flag == BKPTCLR)
                break;
        if (bk==nil) {
            bk = (BKPT *)malloc(sizeof(*bk));
            if (bk == nil)
                error("too many breakpoints");
            bk->nxtbkpt=bkpthead;
            bkpthead=bk;
        }
        /*e: [[subpcs()]] breakpoint case, find unused breakpoint bk or allocate one */

        bk->loc = dot;
        bk->flag = modif == 'b' ? BKPTSET : BKPTTMP;
        bk->initcnt = bk->count = cntval;

        /*s: [[subpcs()]] breakpoint case, set optional breakpoint command */
        check=MAXCOM-1;
        comptr=bk->comm;

        rdc();
        reread();

        do {
            *comptr++ = readchar();
        } while (check-- && lastc!=EOR);
        *comptr='\0';
        if(bk->comm[0] != EOR && cntflg == FALSE)
            bk->initcnt = bk->count = HUGEINT;
        reread();
        if (check)
            return;
        error("bkpt command too long");
        /*e: [[subpcs()]] breakpoint case, set optional breakpoint command */
    /*x: [[subpcs()]] switch modif cases */
    /* delete breakpoint */
    case 'd': 
    case 'D':
        if ((bk=scanbkpt(dot)) == 0)
            error("no breakpoint set");
        bk->flag=BKPTCLR;
        return;
    /*x: [[subpcs()]] switch modif cases */
    /* continue with optional note */
    case 'c': 
    case 'C': 
        if (pid==0)
            error(NOPCS);
        runmode=CONTIN;
        keepnote=defval(1);
        break;
    /*x: [[subpcs()]] switch modif cases */
    case 'S':
        if (pid == 0) {
            setup();
            loopcnt--;
        }
        keepnote=defval(true);

        line = pc2line(rget(cormap, mach->pc));
        n = loopcnt;
        dprint("%s: running\n", symfil);
        flush();
        for (loopcnt = 1; n > 0; loopcnt = 1) {
            r = runpcs(SINGLE, keepnote);
            curr = pc2line(dot);
            if (line != curr) {	/* on a new line of c */
                line = curr;
                n--;
            }
        }
        loopcnt = 0;
        break;
    /*x: [[subpcs()]] switch modif cases */
    /* deal with notes */
    case 'n':	
        if (pid==0)
            error(NOPCS);
        n=defval(-1);
        if(n>=0 && n<nnote){
            nnote--;
            memmove(note[n], note[n+1], (nnote-n)*sizeof(note[0]));
        }
        notes();
        return;
    /*e: [[subpcs()]] switch modif cases */
    default:
        error("bad `:' command");
    }

    if (loopcnt>0) {
        dprint("%s: running\n", symfil);
        flush();
        r = runpcs(runmode, keepnote);
    }
    if (r)
        dprint("breakpoint%16t");
    else
        dprint("stopped at%16t");

Return:
    delbp();
    printpc();
    notes();
}
/*e: function [[subpcs]] */
/*e: db/pcs.c */
