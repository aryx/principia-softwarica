/*s: rc/code.c */
/*s: includes */
#include "rc.h"
#include "exec.h"
#include "fns.h"
#include "getflags.h"
#include "io.h"
/*e: includes */
#include "y.tab.h"

/*s: constant c0 (rc/code.c) */
#define	c0	t->child[0]
/*e: constant c0 (rc/code.c) */
/*s: constant c1 (rc/code.c) */
#define	c1	t->child[1]
/*e: constant c1 (rc/code.c) */
/*s: constant c2 (rc/code.c) */
#define	c2	t->child[2]
/*e: constant c2 (rc/code.c) */
/*s: global [[codep]] */
// idx in codebuf
int codep;
/*e: global [[codep]] */
/*s: global [[ncode]] */
int ncode;
/*e: global [[ncode]] */
/*s: function [[emitf]] */
#define	emitf(x) ((codep!=ncode || morecode()), codebuf[codep].f = (x), codep++)
/*e: function [[emitf]] */
/*s: function [[emiti]] */
#define	emiti(x) ((codep!=ncode || morecode()), codebuf[codep].i = (x), codep++)
/*e: function [[emiti]] */
/*s: function [[emits]] */
#define	emits(x) ((codep!=ncode || morecode()), codebuf[codep].s = (x), codep++)
/*e: function [[emits]] */


// forward decls
void outcode(tree*, bool);
void codeswitch(tree*, bool);
bool iscase(tree*);

/*s: function [[morecode]] */
//@Scheck: used by the macros above (why marked as dead then??? TODO)
int morecode(void)
{
    ncode+=100;
    codebuf = (code *)realloc((char *)codebuf, ncode*sizeof codebuf[0]);
    if(codebuf==nil)
        panic("Can't realloc %d bytes in morecode!", ncode*sizeof(code));
    return OK_0;
}
/*e: function [[morecode]] */

/*s: function [[stuffdot]] */
void
stuffdot(int a)
{
    if(a<0 || codep<=a)
        panic("Bad address %d in stuffdot", a);
    codebuf[a].i = codep;
}
/*e: function [[stuffdot]] */

/*s: function [[compile]] */
//@Scheck: called from syn.y
error0 compile(tree *t)
{
    ncode = 100;
    codep = 0;
    codebuf = (code *)emalloc(ncode*sizeof(code));

    emiti(0);			/* reference count */
    outcode(t, flag['e'] ? true : false);

    /*s: [[compile()]] check nerror */
    if(nerror){
        efree((char *)codebuf);
        return ERROR_0;
    }
    /*e: [[compile()]] check nerror */
    /*s: [[compile()]] after outcode and error management, read heredoc */
    readhere();
    /*e: [[compile()]] after outcode and error management, read heredoc */

    emitf(Xreturn);
    emitf(nil);

    return OK_1;
}
/*e: function [[compile]] */

/*s: function [[cleanhere]] */
void
cleanhere(char *f)
{
    emitf(Xdelhere);
    emits(strdup(f));
}
/*e: function [[cleanhere]] */

/*s: function [[fnstr]] */
char*
fnstr(tree *t)
{
    void *v;
    extern char nl;
    char svnl = nl;
    io *f = openstr();

    nl = ';';
    pfmt(f, "%t", t);
    nl = svnl;
    v = f->strp;
    f->strp = nil;
    closeio(f);
    return v;
}
/*e: function [[fnstr]] */

/*s: function [[outcode]] */
void
outcode(tree *t, bool eflag)
{
    /*s: [[outcode()]] locals */
    int p;
    /*x: [[outcode()]] locals */
    int q;
    /*x: [[outcode()]] locals */
    tree *tt;
    /*e: [[outcode()]] locals */

    if(t==nil)
        return;

    /*s: [[outcode()]] set iflast before switch */
    if(t->type!=NOT && t->type!=';')
        runq->iflast = false;
    /*e: [[outcode()]] set iflast before switch */
    switch(t->type){
    /*s: [[outcode()]] cases */
    case SIMPLE:
        emitf(Xmark);
        outcode(c0, eflag); // the arguments and argv0
        emitf(Xsimple);
        /*s: [[outcode()]] emit Xeflag after Xsimple */
        if(eflag)
            emitf(Xeflag);
        /*e: [[outcode()]] emit Xeflag after Xsimple */
        break;
    /*x: [[outcode()]] cases */
    case ARGLIST:
        outcode(c1, eflag);
        outcode(c0, eflag);
        break;
    /*x: [[outcode()]] cases */
    case WORDS:
        outcode(c1, eflag);
        outcode(c0, eflag);
        break;
    /*x: [[outcode()]] cases */
    case WORD:
        emitf(Xword);
        emits(strdup(t->str));
        break;
    /*x: [[outcode()]] cases */
    case ';':
        outcode(c0, eflag);
        outcode(c1, eflag);
        break;
    /*x: [[outcode()]] cases */
    case ANDAND:
        outcode(c0, false);
        emitf(Xtrue);
        p = emiti(0);
        outcode(c1, eflag);
        stuffdot(p);
        break;
    /*x: [[outcode()]] cases */
    case OROR:
        outcode(c0, false);
        emitf(Xfalse);
        p = emiti(0);
        outcode(c1, eflag);
        stuffdot(p);
        break;
    /*x: [[outcode()]] cases */
    case BANG:
        outcode(c0, eflag);
        emitf(Xbang);
        break;
    /*x: [[outcode()]] cases */
    case TWIDDLE:
        emitf(Xmark);
        outcode(c1, eflag);
        emitf(Xmark);
        outcode(c0, eflag);
        emitf(Xmatch);
        if(eflag)
            emitf(Xeflag);
        break;
    /*x: [[outcode()]] cases */
    case REDIR:
        emitf(Xmark);
        outcode(c0, eflag);
        emitf(Xglob);

        switch(t->rtype){
        /*s: [[outcode()]] when REDIR case, switch redirection type cases */
        case WRITE:
            emitf(Xwrite);
            break;
        /*x: [[outcode()]] when REDIR case, switch redirection type cases */
        case READ:
        case HERE:
            emitf(Xread);
            break;
        /*x: [[outcode()]] when REDIR case, switch redirection type cases */
        case APPEND:
            emitf(Xappend);
            break;
        /*x: [[outcode()]] when REDIR case, switch redirection type cases */
        case RDWR:
            emitf(Xrdwr);
            break;
        /*e: [[outcode()]] when REDIR case, switch redirection type cases */
        }
        emiti(t->fd0);
        outcode(c1, eflag);
        emitf(Xpopredir);
        break;
    /*x: [[outcode()]] cases */
    case PIPE:
        emitf(Xpipe);
        emiti(t->fd0); // 1 in the normal case
        emiti(t->fd1); // 0 in the normal case
        p = emiti(0);
        q = emiti(0);

        // for first child
        outcode(c0, eflag);
        emitf(Xexit);

        // for second child
        stuffdot(p);
        outcode(c1, eflag);
        emitf(Xreturn);
 
        // for parent (rc)
        stuffdot(q);
        emitf(Xpipewait);
        break;
    /*x: [[outcode()]] cases */
    case '&':
        emitf(Xasync);
        p = emiti(0);
        outcode(c0, eflag);
        emitf(Xexit);
        stuffdot(p);
        break;
    /*x: [[outcode()]] cases */
    case IF:
        outcode(c0, false);
        emitf(Xif);
        p = emiti(0);
        outcode(c1, eflag);
        emitf(Xwastrue);
        stuffdot(p);
        break;
    /*x: [[outcode()]] cases */
    case NOT:
        /*s: [[outcode()]] when NOT, sanity check last command was an if */
        if(!runq->iflast)
            yyerror("`if not' does not follow `if(...)'");
        /*e: [[outcode()]] when NOT, sanity check last command was an if */
        emitf(Xifnot);
        p = emiti(0);
        outcode(c0, eflag);
        stuffdot(p);
        break;
    /*x: [[outcode()]] cases */
    case WHILE:
        q = codep;
        outcode(c0, false);
        if(q==codep)
            emitf(Xsettrue);	/* empty condition == while(true) */
        emitf(Xtrue);
        p = emiti(0);
        outcode(c1, eflag);
        emitf(Xjump);
        emiti(q);
        stuffdot(p);
        break;
    /*x: [[outcode()]] cases */
    case FOR:
        emitf(Xmark);
        if(c1){
            outcode(c1, eflag);
            emitf(Xglob);
        }
        else{
            emitf(Xmark);
            emitf(Xword);
            emits(strdup("*"));
            emitf(Xdol);
        }
        emitf(Xmark);		/* dummy value for Xlocal */
        emitf(Xmark);
        outcode(c0, eflag);
        emitf(Xlocal);
        p = emitf(Xfor);
        q = emiti(0);
        outcode(c2, eflag);
        emitf(Xjump);
        emiti(p);
        stuffdot(q);
        emitf(Xunlocal);
        break;
    /*x: [[outcode()]] cases */
    case SWITCH:
        codeswitch(t, eflag);
        break;
    /*x: [[outcode()]] cases */
    case PAREN:
        outcode(c0, eflag);
        break;
    /*x: [[outcode()]] cases */
    case PCMD:
    case BRACE:
        outcode(c0, eflag);
        break;
    /*x: [[outcode()]] cases */
    case FN:
        emitf(Xmark);
        outcode(c0, eflag);
        if(c1){
            emitf(Xfn);
            p = emiti(0);
            emits(fnstr(c1));
            outcode(c1, eflag); // body of the function
            emitf(Xunlocal);	/* get rid of $* */ //$
            emitf(Xreturn);
            stuffdot(p);
        }
        else
            emitf(Xdelfn);
        break;
    /*x: [[outcode()]] cases */
    case '=':
        tt = t;
        for(;t && t->type=='='; t = c2);

        if(t){					/* var=value cmd */
            for(t = tt;t->type=='=';t = c2){
                emitf(Xmark);
                outcode(c1, eflag);
                emitf(Xmark);
                outcode(c0, eflag);
                emitf(Xlocal);		/* push var for cmd */
            }
            outcode(t, eflag);		/* gen. code for cmd */
            for(t = tt; t->type == '='; t = c2)
                emitf(Xunlocal);	/* pop var */
        }
        else{					/* var=value */
            for(t = tt;t;t = c2){
                emitf(Xmark);
                outcode(c1, eflag);
                emitf(Xmark);
                outcode(c0, eflag);
                emitf(Xassign);	/* set var permanently */
            }
        }
        t = tt;	/* so tests below will work */
        break;
    /*x: [[outcode()]] cases */
    case '$': //$
        emitf(Xmark);
        outcode(c0, eflag);
        emitf(Xdol);
        break;
    /*x: [[outcode()]] cases */
    case COUNT:
        emitf(Xmark);
        outcode(c0, eflag);
        emitf(Xcount);
        break;
    /*x: [[outcode()]] cases */
    case SUB:
        emitf(Xmark);
        outcode(c0, eflag);
        emitf(Xmark);
        outcode(c1, eflag);
        emitf(Xsub);
        break;
    /*x: [[outcode()]] cases */
    case '^':
        emitf(Xmark);
        outcode(c1, eflag);
        emitf(Xmark);
        outcode(c0, eflag);
        emitf(Xconc);
        break;
    /*x: [[outcode()]] cases */
    case SUBSHELL:
        emitf(Xsubshell);
        p = emiti(0);
        outcode(c0, eflag);
        emitf(Xexit);
        stuffdot(p);
        if(eflag)
            emitf(Xeflag);
        break;
    /*x: [[outcode()]] cases */
    case DUP:
        if(t->rtype==DUPFD){
            emitf(Xdup);
            emiti(t->fd0);
            emiti(t->fd1);
        }
        else{
            emitf(Xclose);
            emiti(t->fd0);
        }
        outcode(c1, eflag);
        emitf(Xpopredir);
        break;
    /*x: [[outcode()]] cases */
    case PIPEFD:
        emitf(Xpipefd);
        emiti(t->rtype);
        p = emiti(0);
        outcode(c0, eflag);
        emitf(Xexit);
        stuffdot(p);
        break;
    /*x: [[outcode()]] cases */
    case '`':
        emitf(Xbackq);
        p = emiti(0);
        outcode(c0, false);
        emitf(Xexit);
        stuffdot(p);
        break;
    /*x: [[outcode()]] cases */
    case '"':
        emitf(Xmark);
        outcode(c0, eflag);
        emitf(Xqdol);
        break;
    /*e: [[outcode()]] cases */
    default:
        pfmt(err, "bad type %d in outcode\n", t->type);
        break;
    }
    /*s: [[outcode()]] set iflast after switch */
    if(t->type!=NOT && t->type!=';')
        runq->iflast = t->type==IF;
    else 
       if(c0) 
           runq->iflast = c0->type==IF;
    /*e: [[outcode()]] set iflast after switch */
}
/*e: function [[outcode]] */
/*s: function [[codeswitch]] */
/*
 * switch code looks like this:
 *	Xmark
 *	(get switch value)
 *	Xjump	1f
 * out:	Xjump	leave
 * 1:	Xmark
 *	(get case values)
 *	Xcase	1f
 *	(commands)
 *	Xjump	out
 * 1:	Xmark
 *	(get case values)
 *	Xcase	1f
 *	(commands)
 *	Xjump	out
 * 1:
 * leave:
 *	Xpopm
 */
void
codeswitch(tree *t, bool eflag)
{
    int leave;		/* patch jump address to leave switch */
    int out;		/* jump here to leave switch */
    int nextcase;	/* patch jump address to next case */
    tree *tt;

    // c1 is BRACE { ; ; ; }
    if(c1->child[0]==nil
    || c1->child[0]->type != ';'
    || !iscase(c1->child[0]->child[0])){
        yyerror("case missing in switch");
        return;
    }

    emitf(Xmark);
    outcode(c0, eflag);
    emitf(Xjump);

    nextcase = emiti(0);
    out = emitf(Xjump);
    leave = emiti(0);

    stuffdot(nextcase);

    // from now on c0, c1, ... refer to this new t
    t = c1->child[0];
    while(t->type==';'){
        tt = c1;
        emitf(Xmark);
        for(t = c0->child[0];t->type==ARGLIST;t = c0) 
            outcode(c1, eflag);
        emitf(Xcase);
        nextcase = emiti(0);
        t = tt;
        for(;;){
            if(t->type==';'){
                if(iscase(c0)) 
                    break;
                outcode(c0, eflag);
                t = c1;
            }
            else{
                if(!iscase(t)) 
                    outcode(t, eflag);
                break;
            }
        }
        emitf(Xjump);
        emiti(out);
        stuffdot(nextcase);
    }
    stuffdot(leave);
    emitf(Xpopm);
}
/*e: function [[codeswitch]] */

/*s: function [[iscase]] */
bool
iscase(tree *t)
{
    if(t->type!=SIMPLE)
        return false;
    do { t = c0; } while(t->type==ARGLIST);
    return t->type==WORD && !t->quoted && strcmp(t->str, "case")==0;
}
/*e: function [[iscase]] */

/*s: function [[codecopy]] */
code*
codecopy(code *cp)
{
    cp[0].i++;
    return cp;
}
/*e: function [[codecopy]] */

/*s: function [[codefree]] */
void
codefree(code *cp)
{
    code *p;
    // check ref count
    if(--cp[0].i != 0)
        return;

    for(p = cp+1; p->f; p++){
        /*s: [[codefree()]] in loop over code [[cp]], switch bytecode cases */
        if(p->f==Xfalse || p->f==Xtrue
        || p->f==Xread || p->f==Xwrite || p->f==Xrdwr
        || p->f==Xappend || p->f==Xclose 
        || p->f==Xasync || p->f==Xbackq || p->f==Xcase 
        || p->f==Xfor || p->f==Xjump
        || p->f==Xsubshell)
            p++;
        /*x: [[codefree()]] in loop over code [[cp]], switch bytecode cases */
        else if(p->f==Xword || p->f==Xdelhere) 
                 efree((++p)->s);
        /*x: [[codefree()]] in loop over code [[cp]], switch bytecode cases */
        else if(p->f==Xpipe) 
                 p+=4;
        /*x: [[codefree()]] in loop over code [[cp]], switch bytecode cases */
        else if(p->f==Xfn){
                 efree(p[2].s);
                 p+=2;
              }
        /*x: [[codefree()]] in loop over code [[cp]], switch bytecode cases */
        else if(p->f==Xdup || p->f==Xpipefd) 
                 p+=2;
        /*e: [[codefree()]] in loop over code [[cp]], switch bytecode cases */
    }
    efree((char *)cp);
}
/*e: function [[codefree]] */
/*e: rc/code.c */
