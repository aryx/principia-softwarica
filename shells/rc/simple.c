/*s: rc/simple.c */
/*
 * Maybe `simple' is a misnomer.
 */
/*s: includes */
#include "rc.h"
#include "exec.h"
#include "fns.h"
#include "getflags.h"
#include "io.h"
/*e: includes */

// forward decls
void execfunc(var*);

/*s: function [[exitnext]] */
/*
 * Search through the following code to see if we're just going to exit.
 */
int
exitnext(void){
    union Code *c = &runq->code[runq->pc];
    while(c->f==Xpopredir) 
        c++;
    return c->f==Xexit;
}
/*e: function [[exitnext]] */

/*s: function [[Xsimple]] */
void
Xsimple(void)
{
    word *a;
    thread *p = runq;
    int pid;
    /*s: [[Xsimple()]] other locals */
    var *v;
    /*x: [[Xsimple()]] other locals */
    struct Builtin *bp;
    /*e: [[Xsimple()]] other locals */

    /*s: [[Xsimple()]] initializations, [[globlist()]] */
    globlist();
    /*e: [[Xsimple()]] initializations, [[globlist()]] */

    a = runq->argv->words;
    /*s: [[Xsimple()]] sanity check a */
    if(a==nil){
        Xerror1("empty argument list");
        return;
    }
    /*e: [[Xsimple()]] sanity check a */

    /*s: [[Xsimple()]] if -x */
    if(flag['x'])
        pfmt(err, "%v\n", p->argv->words); /* wrong, should do redirs */
    /*e: [[Xsimple()]] if -x */

    /*s: [[Xsimple()]] if argv0 is a function */
    v = gvlook(a->word);
    if(v->fn)
        execfunc(v);
    /*e: [[Xsimple()]] if argv0 is a function */
    else{
        /*s: [[Xsimple()]] if argv0 is a builtin */
        /*s: [[Xsimple()]] if argv0 is the builtin keyword */
        if(strcmp(a->word, "builtin")==0){
            if(count(a)==1){
                pfmt(err, "builtin: empty argument list\n");
                setstatus("empty arg list");
                poplist();
                return;
            }
            a = a->next;
            popword();
        }
        /*e: [[Xsimple()]] if argv0 is the builtin keyword */
        for(bp = builtins;bp->name;bp++)
            if(strcmp(a->word, bp->name)==0){
                (*bp->fnc)();
                return;
            }
        /*e: [[Xsimple()]] if argv0 is a builtin */
        /*s: [[Xsimple()]] if exitnext() */
        if(exitnext()){
            /* fork and wait is redundant */
            pushword("exec");
            execexec();
            Xexit();
        }
        /*e: [[Xsimple()]] if exitnext() */
        else{
            flush(err);
            Updenv();	/* necessary so changes don't go out again */
            if((pid = execforkexec()) < 0){
                Xerror("try again");
                return;
            }

            /* interrupts don't get us out */
            poplist();
            while(Waitfor(pid, 1) < 0)
                ;
        }
    }
}
/*e: function [[Xsimple]] */

/*s: function [[doredir]] */
void
doredir(redir *rp)
{
    if(rp){
        // recurse first, so do them in the reverse order of the list
        doredir(rp->next);

        switch(rp->type){
        /*s: [[doredir()]] switch redir type cases */
        case ROPEN:
            if(rp->from != rp->to){
                Dup(rp->from, rp->to);
                close(rp->from);
            }
            break;
        /*x: [[doredir()]] switch redir type cases */
        case RDUP:
            Dup(rp->from, rp->to);
            break;
        /*x: [[doredir()]] switch redir type cases */
        case RCLOSE:
            close(rp->from);
            break;
        /*e: [[doredir()]] switch redir type cases */
        }
    }
}
/*e: function [[doredir]] */

/*s: function [[execexec]] */
void
execexec(void)
{
    popword();	/* "exec" */
    /*s: [[execexec()]] sanity check arguments */
    if(runq->argv->words==nil){
        Xerror1("empty argument list");
        return;
    }
    /*e: [[execexec()]] sanity check arguments */
    /*s: [[execexec()]] perform the redirections */
    doredir(runq->redir);
    /*e: [[execexec()]] perform the redirections */

    Execute(runq->argv->words, searchpath(runq->argv->words->word));
    // should not be reached! unless command did not exist
    poplist();
}
/*e: function [[execexec]] */

/*s: function [[execfunc]] */
void
execfunc(var *func)
{
    word *starval;

    popword();
    starval = runq->argv->words;
    runq->argv->words = nil;
    poplist();
    start(func->fn, func->pc, runq->local);
    runq->local = newvar(strdup("*"), runq->local);
    runq->local->val = starval;
    runq->local->changed = true;
}
/*e: function [[execfunc]] */

/*s: global [[rdcmds]] */
union Code rdcmds[4];
/*e: global [[rdcmds]] */

/*s: function [[execcmds]] */
void
execcmds(io *f)
{
    static bool first = true;
    if(first){
        rdcmds[0].i = 1;
        rdcmds[1].f = Xrdcmds;
        rdcmds[2].f = Xreturn;
        first = false;
    }

    start(rdcmds, 1, runq->local);
    runq->cmdfd = f;
    runq->iflast = false;
}
/*e: function [[execcmds]] */

/*e: rc/simple.c */
