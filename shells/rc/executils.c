/*s: rc/executils.c */
/*s: includes */
#include "rc.h"
#include "exec.h"
#include "fns.h"
#include "getflags.h"
#include "io.h"
/*e: includes */

/*s: global [[argv0]] */
/*
 * Start executing the given code at the given pc with the given redirection
 */
char *argv0="rc";
/*e: global [[argv0]] */

/*s: function [[start]] */
void
start(code *c, int pc, var *local)
{
    struct Thread *p = new(struct Thread);

    p->code = codecopy(c);
    p->pc = pc;

    p->argv = nil;
    p->local = local;

    p->cmdfile = nil;
    p->cmdfd = nil;
    p->lineno = 1;
    p->eof = false;
    p->iflag = false;

    /*s: [[start()]] set redir */
    p->redir = p->startredir = runq ? runq->redir : nil;
    /*e: [[start()]] set redir */

    // add_stack(runq, p)
    p->ret = runq;
    runq = p;
}
/*e: function [[start]] */


/*s: function [[pushword]] */
void
pushword(char *wd)
{
    if(runq->argv==nil)
        panic("pushword but no argv!", 0);
    runq->argv->words = newword(wd, runq->argv->words);
}
/*e: function [[pushword]] */

/*s: function [[popword]] */
void
popword(void)
{
    word *p;
    /*s: [[popword()]] sanity check argv */
    if(runq->argv==nil)
        panic("popword but no argv!", 0);
    /*e: [[popword()]] sanity check argv */
    p = runq->argv->words;
    /*s: [[popword()]] sanity check argv words */
    if(p==nil)
        panic("popword but no word!", 0);
    /*e: [[popword()]] sanity check argv words */
    runq->argv->words = p->next;
    efree(p->word);
    efree((char *)p);
}
/*e: function [[popword]] */

/*s: function [[pushlist]] */
void
pushlist(void)
{
    list *p = new(list);

    // add_list(p, runq->argv)
    p->next = runq->argv;
    p->words = nil;
    runq->argv = p;
}
/*e: function [[pushlist]] */

/*s: function [[poplist]] */
void
poplist(void)
{
    list *p = runq->argv;
    if(p==nil)
        panic("poplist but no argv", 0);
    freelist(p->words);
    runq->argv = p->next;
    efree((char *)p);
}
/*e: function [[poplist]] */


/*s: function [[pushredir]] */
void
pushredir(int type, int from, int to)
{
    redir * rp = new(redir);
    rp->type = type;
    rp->from = from;
    rp->to = to;

    // add_list(runq->redir, rp)
    rp->next = runq->redir;
    runq->redir = rp;
}
/*e: function [[pushredir]] */


/*s: function [[Xerror]] */
void
Xerror(char *s)
{
    if(strcmp(argv0, "rc")==0 || strcmp(argv0, "/bin/rc")==0)
        pfmt(err, "rc: %s: %r\n", s);
    else
        pfmt(err, "rc (%s): %s: %r\n", argv0, s);
    flush(err);
    setstatus("error");

    while(!runq->iflag) 
        Xreturn();
}
/*e: function [[Xerror]] */

/*s: function [[Xerror1]] */
void
Xerror1(char *s)
{
    if(strcmp(argv0, "rc")==0 || strcmp(argv0, "/bin/rc")==0)
        pfmt(err, "rc: %s\n", s);
    else
        pfmt(err, "rc (%s): %s\n", argv0, s);
    flush(err);
    setstatus("error");

    while(!runq->iflag) 
        Xreturn();
}
/*e: function [[Xerror1]] */


/*s: function [[turfredir]] */
void
turfredir(void)
{
    while(runq->redir != runq->startredir)
        Xpopredir();
}
/*e: function [[turfredir]] */

/*s: function [[Xpopredir]] */
void
Xpopredir(void)
{
    struct Redir *rp = runq->redir;

    if(rp==nil)
        panic("turfredir null!", 0);

    // pop_list(runq->redir);
    runq->redir = rp->next;

    if(rp->type==ROPEN)
        close(rp->from);

    efree((char *)rp);
}
/*e: function [[Xpopredir]] */

/*s: function [[Xreturn]] */
void
Xreturn(void)
{
    struct Thread *p = runq;

    /*s: [[Xreturn()]] pop the redirections from this thread */
    turfredir();
    /*e: [[Xreturn()]] pop the redirections from this thread */
    // free p
    while(p->argv) 
        poplist();
    codefree(p->code);
    // pop(runq)
    runq = p->ret;
    efree((char *)p);

    if(runq==nil)
        Exit(getstatus());
}
/*e: function [[Xreturn]] */

/*e: rc/executils.c */
