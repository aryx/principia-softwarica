/*s: rc/trap.c */
/*s: includes */
#include "rc.h"
#include "exec.h"
#include "fns.h"
#include "getflags.h"
#include "io.h"
/*e: includes */

// NSIG, SIGINT, SIGQUIT are back in rc.h (under an ifdef for plan9)

// was in exec.h
/*s: global [[ntrap]] */
int ntrap;				/* number of outstanding traps */
/*e: global [[ntrap]] */
/*s: global [[trap]] */
int trap[NSIG];				/* number of outstanding traps per type */
/*e: global [[trap]] */

// signame, syssigname, interrputed are back in plan9.c
// notifyf(), Trapinit(), Eintr(), Noerror() are back in plan9.c
extern char *signame[];

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
            Exit(getstatus(), __LOC__);
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
            Exit(getstatus(), __LOC__);
    }
}
/*e: function [[dotrap]] */
/*e: rc/trap.c */
