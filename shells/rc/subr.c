/*s: rc/subr.c */
#include "rc.h"
#include "exec.h"
#include "io.h"
#include "fns.h"

/*s: function emalloc */
void *
emalloc(long n)
{
    void *p = Malloc(n);

    if(p==nil)
        panic("Can't malloc %d bytes", n);
/*	if(err){ pfmt(err, "malloc %d->%p\n", n, p); flush(err); } /**/
    return p;
}
/*e: function emalloc */

/*s: function efree */
void
efree(void *p)
{
    if(p)
        free(p);
    else pfmt(err, "free 0\n");
}
/*e: function efree */
extern bool lastword;
extern bool lastdol;

/*s: function yyerror */
void
yyerror(char *m)
{
    pfmt(err, "rc: ");
    if(runq->cmdfile && !runq->iflag)
        pfmt(err, "%s:%d: ", runq->cmdfile, runq->lineno);
    else if(runq->cmdfile)
        pfmt(err, "%s: ", runq->cmdfile);
    else if(!runq->iflag)
        pfmt(err, "line %d: ", runq->lineno);

    if(tok[0] && tok[0]!='\n')
        pfmt(err, "token %q: ", tok);
    pfmt(err, "%s\n", m);
    flush(err);
    lastword = false;
    lastdol = false;

    while(lastc!='\n' && lastc!=EOF) 
        advance();

    nerror++;
    setvar("status", newword(m, (word *)nil));
}
/*e: function yyerror */
/*s: global bp */
char *bp;
/*e: global bp */

/*s: function iacvt */
static void
iacvt(int n)
{
    if(n<0){
        *bp++='-';
        n=-n;	/* doesn't work for n==-inf */
    }
    if(n/10)
        iacvt(n/10);
    *bp++=n%10+'0';
}
/*e: function iacvt */

/*s: function inttoascii */
void
inttoascii(char *s, long n)
{
    bp = s;
    iacvt(n);
    *bp='\0';
}
/*e: function inttoascii */

/*s: function panic */
void
panic(char *s, int n)
{
    pfmt(err, "rc: ");
    pfmt(err, s, n);
    pchr(err, '\n');
    flush(err);
    Abort();
}
/*e: function panic */
/*e: rc/subr.c */
