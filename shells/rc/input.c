/*s: rc/input.c */
/*s: includes */
#include "rc.h"
#include "exec.h"
#include "fns.h"
#include "getflags.h"
#include "io.h"
/*e: includes */
#include "y.tab.h"

// was in lex.c

/*s: global [[future]] */
int future = EOF;
/*e: global [[future]] */
/*s: global [[doprompt]] */
bool doprompt = true;
/*e: global [[doprompt]] */
/*s: global [[inquote]] */
bool inquote;
/*e: global [[inquote]] */
/*s: global [[incomm]] */
bool incomm;
/*e: global [[incomm]] */

// was in rc.h
/*s: global [[lastc]] */
int lastc;
/*e: global [[lastc]] */

// forward decl
int getnext(void);

/*s: function [[nextc]] */
/// yylex | advance | skipnl | nextis -> <>
/*
 * Look ahead in the input stream
 */
int
nextc(void)
{
    if(future==EOF)
        future = getnext();
    return future;
}
/*e: function [[nextc]] */
/*s: function [[advance]] */
/// nextis -> <>
/*
 * Consume the lookahead character.
 */
int
advance(void)
{
    int c = nextc();
    /*s: [[advance()]] save future in lastc */
    lastc = future;
    /*e: [[advance()]] save future in lastc */
    future = EOF;
    return c;
}
/*e: function [[advance]] */
/*s: function [[getnext]] */
/// yylex -> nextc -> <>
/*
 * read a character from the input stream
 */	
int
getnext(void)
{
    int c;
    /*s: [[getnext()]] other locals */
    static int peekc = EOF;
    /*e: [[getnext()]] other locals */

    /*s: [[getnext()]] before read, peekc handling */
    if(peekc!=EOF){
        c = peekc;
        peekc = EOF;
        return c;
    }
    /*e: [[getnext()]] before read, peekc handling */
    /*s: [[getnext()]] before read, return if already at EOF */
    if(runq->eof)
        return EOF;
    /*e: [[getnext()]] before read, return if already at EOF */
    /*s: [[getnext()]] before read, prompt management */
    if(doprompt)
        // pprompt() internally set doprompt back to false at the end
        pprompt();
    /*e: [[getnext()]] before read, prompt management */

    c = rchr(runq->cmdfd);

    /*s: [[getnext()]] after read, handle backslash */
    if(!inquote && c=='\\'){

        c = rchr(runq->cmdfd);

        if(c=='\n' && !incomm){		/* don't continue a comment */
            /*s: [[getnext()]] when backslash and newline, set doprompt */
            doprompt = true;
            /*e: [[getnext()]] when backslash and newline, set doprompt */
            c=' ';
        }
        else{
            peekc = c;
            c='\\';
        }
    }
    /*e: [[getnext()]] after read, handle backslash */
    /*s: [[getnext()]] after read, prompt management */
    doprompt = doprompt || c=='\n' || c==EOF;
    /*e: [[getnext()]] after read, prompt management */
    /*s: [[getnext()]] after read, if EOF */
    if(c==EOF)
        runq->eof = true;
    /*e: [[getnext()]] after read, if EOF */
    /*s: [[getnext()]] after read, if not EOF but verbose mode, print character read */
    else 
        if(flag['V'] || ndot>=2 && flag['v'])
            pchr(err, c);
    /*e: [[getnext()]] after read, if not EOF but verbose mode, print character read */

    return c;
}
/*e: function [[getnext]] */

/*s: function [[pprompt]] */
void
pprompt(void)
{
    var *prompt;

    if(runq->iflag){
        // printing the prompt
        pstr(err, promptstr);
        flush(err);

        // set promptstr for the next pprompt()
        prompt = vlook("prompt");
        if(prompt->val && prompt->val->next)
            promptstr = prompt->val->next->word;
        else
            promptstr="\t";
    }
    runq->lineno++;
    doprompt = false;
}
/*e: function [[pprompt]] */


/*s: function [[nextis]] */
/// yylex -> <>
bool
nextis(int c)
{
    if(nextc()==c){
        advance();
        return true;
    }
    return false;
}
/*e: function [[nextis]] */


extern unsigned hash(char *as, int n);

// was in tree.c

/*s: function [[token]] */
tree*
token(char *str, int type)
{
    tree *t = newtree();

    t->type = type;
    t->str = strdup(str);
    return t;
}
/*e: function [[token]] */

// was in var.c

/*s: constant [[NKW]] */
#define	NKW	30
/*e: constant [[NKW]] */
/*s: struct [[Kw]] */
struct Kw {
    char *name;
    int type;

    struct Kw *next;
};
/*e: struct [[Kw]] */
/*s: global [[kw]] */
struct Kw *kw[NKW];
/*e: global [[kw]] */

/*s: function [[kenter]] */
void
kenter(int type, char *name)
{
    int h = hash(name, NKW);
    struct Kw *p = new(struct Kw);
    p->type = type;
    p->name = name;
    p->next = kw[h];
    kw[h] = p;
}
/*e: function [[kenter]] */

/*s: function [[kinit]] */
void
kinit(void)
{
    kenter(FOR, "for");
    kenter(IN, "in");
    kenter(WHILE, "while");
    kenter(IF, "if");
    kenter(NOT, "not");
    kenter(SWITCH, "switch");
    kenter(FN, "fn");

    kenter(TWIDDLE, "~");
    kenter(BANG, "!");
    kenter(SUBSHELL, "@");
}
/*e: function [[kinit]] */

/*s: function [[klook]] */
tree*
klook(char *name)
{
    struct Kw *p;
    tree *t = token(name, WORD); // default if actually not a keyword

    for(p = kw[hash(name, NKW)];p;p = p->next)
        if(strcmp(p->name, name)==0){
            t->type = p->type;
            break;
        }
    return t;
}
/*e: function [[klook]] */
/*e: rc/input.c */
