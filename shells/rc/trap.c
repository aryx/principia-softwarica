/*s: rc/trap.c */
/*s: includes */
#include "rc.h"
#include "exec.h"
#include "fns.h"
#include "getflags.h"
#include "io.h"
/*e: includes */

// was in rc.h (under an ifdef for plan9)
/*s: constant [[NSIG]] */
#define	NSIG	32
/*e: constant [[NSIG]] */
/*s: constant [[SIGINT]] */
#define	SIGINT	2
/*e: constant [[SIGINT]] */
/*s: constant [[SIGQUIT]] */
#define	SIGQUIT	3
/*e: constant [[SIGQUIT]] */

// was in exec.h
/*s: global [[ntrap]] */
int ntrap;				/* number of outstanding traps */
/*e: global [[ntrap]] */
/*s: global [[trap]] */
int trap[NSIG];				/* number of outstanding traps per type */
/*e: global [[trap]] */

// was in plan9.c
/*s: global [[signame]] */
char *signame[] = {
    "sigexit",	
    "sighup",	
    "sigint",	
    "sigquit",
    "sigalrm",	
    "sigkill",	
    "sigfpe",	
    "sigterm",
    0
};
/*e: global [[signame]] */
/*s: global [[syssigname]] */
char *syssigname[] = {
    "exit",		/* can't happen */
    "hangup",
    "interrupt",
    "quit",		/* can't happen */
    "alarm",
    "kill",
    "sys: fp: ",
    "term",
    0
};
/*e: global [[syssigname]] */

/*s: global [[interrupted]] */
bool interrupted = false;
/*e: global [[interrupted]] */

/*s: function [[notifyf]] */
void
notifyf(void*, char *s)
{
    int i;
    for(i = 0;syssigname[i];i++) 
     if(strncmp(s, syssigname[i], strlen(syssigname[i]))==0){
        if(strncmp(s, "sys: ", 5)!=0) 
            interrupted = true;
        goto Out;
    }
    pfmt(err, "rc: note: %s\n", s);
    noted(NDFLT);
    return;
Out:
    if(strcmp(s, "interrupt")!=0 || trap[i]==0){
        trap[i]++;
        ntrap++;
    }
    if(ntrap>=32){	/* rc is probably in a trap loop */
        pfmt(err, "rc: Too many traps (trap %s), aborting\n", s);
        abort();
    }
    noted(NCONT);
}
/*e: function [[notifyf]] */

/*s: function [[Trapinit]] */
void
Trapinit(void)
{
    notify(notifyf);
}
/*e: function [[Trapinit]] */

/*s: function [[Eintr]] */
bool
Eintr(void)
{
    return interrupted;
}
/*e: function [[Eintr]] */

/*s: function [[Noerror]] */
void
Noerror(void)
{
    interrupted = false;
}
/*e: function [[Noerror]] */

// generic part independent of plan9
/*s: function [[dotrap]] */
void
dotrap(void)
{
    int i;
    struct Var *trapreq;
    struct Word *starval;

    starval = vlook("*")->val;
    while(ntrap) 
     for(i = 0;i!=NSIG;i++) 
      while(trap[i]){
        --trap[i];
        --ntrap;
        if(getpid()!=mypid) 
            Exit(getstatus());
        trapreq = vlook(signame[i]);
        if(trapreq->fn){
            start(trapreq->fn, trapreq->pc, (struct Var *)nil);
            runq->local = newvar(strdup("*"), runq->local);
            runq->local->val = copywords(starval, (struct Word *)nil);
            runq->local->changed = true;
            runq->redir = runq->startredir = nil;
        }
        else if(i==SIGINT || i==SIGQUIT){
            /*
             * run the stack down until we uncover the
             * command reading loop.  Xreturn will exit
             * if there is none (i.e. if this is not
             * an interactive rc.)
             */
            while(!runq->iflag) 
                Xreturn();
        }
        else 
            Exit(getstatus());
    }
}
/*e: function [[dotrap]] */
/*e: rc/trap.c */
