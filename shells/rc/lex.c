/*s: rc/lex.c */
/*s: includes */
#include "rc.h"
#include "getflags.h"
#include "exec.h"
#include "io.h"
#include "fns.h"
/*e: includes */
#include "x.tab.h"

int getnext(void);

/*s: constant [[NTOK]] */
#define	NTOK	8192		/* maximum bytes in a word (token) */
/*e: constant [[NTOK]] */

// was used by subr.c
/*s: global [[lastdol]] */
bool lastdol;	/* was the last token read '$' or '$#' or '"'? */
/*e: global [[lastdol]] */
/*s: global [[lastword]] */
// used also by syn.y
bool lastword;	/* was the last token read a word or compound word terminator? */
/*e: global [[lastword]] */
// was in rc.h
/*s: global [[tok]] */
char tok[NTOK + UTFmax];
/*e: global [[tok]] */
/*s: global [[lastc]] */
int lastc;
/*e: global [[lastc]] */

/*s: function [[wordchr]] */
int
wordchr(int c)
{
    return !strchr("\n \t#;&|^$=`'{}()<>", c) && c!=EOF;
}
/*e: function [[wordchr]] */

/*s: function [[idchr]] */
int
idchr(int c)
{
    /*
     * Formerly:
     * return 'a'<=c && c<='z' || 'A'<=c && c<='Z' || '0'<=c && c<='9'
     *	|| c=='_' || c=='*';
     */
    return c > ' ' && !strchr("!\"#$%&'()+,-./:;<=>?@[\\]^`{|}~", c);
}
/*e: function [[idchr]] */

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

/*s: function [[nextc]] */
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

    /*s: [[getnext()]] peekc handling */
    if(peekc!=EOF){
        c = peekc;
        peekc = EOF;
        return c;
    }
    /*e: [[getnext()]] peekc handling */
    /*s: [[getnext()]] return if already at EOF */
    if(runq->eof)
        return EOF;
    /*e: [[getnext()]] return if already at EOF */
    /*s: [[getnext()]] prompt management before reading the character */
    if(doprompt)
        // pprompt() internally set doprompt back to false at the end
        pprompt();
    /*e: [[getnext()]] prompt management before reading the character */

    c = rchr(runq->cmdfd);

    /*s: [[getnext()]] handle backslash */
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
    /*e: [[getnext()]] handle backslash */
    /*s: [[getnext()]] prompt management after the character is read */
    doprompt = doprompt || c=='\n' || c==EOF;
    /*e: [[getnext()]] prompt management after the character is read */
    /*s: [[getnext()]] if character read is EOF */
    if(c==EOF)
        runq->eof = true;
    /*e: [[getnext()]] if character read is EOF */
    /*s: [[getnext()]] if not EOF but verbose mode, print character read */
    else 
        if(flag['V'] || ndot>=2 && flag['v'])
            pchr(err, c);
    /*e: [[getnext()]] if not EOF but verbose mode, print character read */

    return c;
}
/*e: function [[getnext]] */

/*s: function [[yyerror]] */
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
/*e: function [[yyerror]] */


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

/*s: function [[skipwhite]] */
void
skipwhite(void)
{
    int c;
    for(;;){
        c = nextc();
        /* Why did this used to be  if(!inquote && c=='#') ?? */
        if(c=='#'){
            incomm = true;
            for(;;){
                c = nextc();
                if(c=='\n' || c==EOF) {
                    incomm = false;
                    break;
                }
                advance();
            }
        }
        if(c==' ' || c=='\t')
            advance();
        else 
            return;
    }
}
/*e: function [[skipwhite]] */

/*s: function [[skipnl]] */
void
skipnl(void)
{
    int c;
    for(;;){
        skipwhite();
        c = nextc();
        if(c!='\n')
            return;
        // consume the newline
        advance();
    }
}
/*e: function [[skipnl]] */

/*s: function [[nextis]] */
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

/*s: function [[addtok]] */
char*
addtok(char *p, int val)
{
    if(p==nil)
        return nil;
    if(p >= &tok[NTOK]){
        *p = '\0';
        yyerror("token buffer too short");
        return nil;
    }
    *p++=val;
    return p;
}
/*e: function [[addtok]] */

/*s: function [[addutf]] */
char*
addutf(char *p, int c)
{
    uchar b, m;
    int i;

    p = addtok(p, c);	/* 1-byte UTF runes are special */
    if(c < Runeself)
        return p;

    m = 0xc0;
    b = 0x80;
    for(i=1; i < UTFmax; i++){
        if((c&m) == b)
            break;
        p = addtok(p, advance());
        b = m;
        m = (m >> 1)|0x80;
    }
    return p;
}
/*e: function [[addutf]] */

/*s: function [[yylex]] */
//@Scheck: called from yyparse()
int yylex(void)
{
    int c;
    /*s: [[yylex()]] other locals */
    char *w = tok;
    /*x: [[yylex()]] other locals */
    struct Tree *t;
    /*x: [[yylex()]] other locals */
    int d = nextc();
    /*e: [[yylex()]] other locals */

    /*s: [[yylex()]] hack for SUB */
    /*
     * Embarassing sneakiness:  if the last token read was a quoted or unquoted
     * WORD then we alter the meaning of what follows.  If the next character
     * is `(', we return SUB (a subscript paren) and consume the `('. Otherwise,
     * if the next character is the first character of a simple or compound word,
     * we insert a `^' before it.
     */
    if(lastword){
        lastword = false;
        if(d=='('){
            advance();
            strcpy(tok, "( [SUB]");
            return SUB;
        }
        if(wordchr(d) || d=='\'' || d=='`' || d=='$' || d=='"'){
            strcpy(tok, "^");
            return '^';
        }
    }
    /*e: [[yylex()]] hack for SUB */
    /*s: [[yylex()]] initialisations */
    yylval.tree = nil;
    /*x: [[yylex()]] initialisations */
    inquote = false;
    /*e: [[yylex()]] initialisations */

    skipwhite();

    switch(c = advance()){
    /*s: [[yylex()]] switch c cases */
    case EOF:
        lastdol = false;
        strcpy(tok, "EOF");
        return EOF;
    /*x: [[yylex()]] switch c cases */
    case '&':
        lastdol = false;
        if(nextis('&')){
            skipnl();
            strcpy(tok, "&&");
            return ANDAND;
        }
        strcpy(tok, "&");
        return '&';
    /*x: [[yylex()]] switch c cases */
    case '$':
        lastdol = true;
        if(nextis('#')){
            strcpy(tok, "$#");
            return COUNT;
        }
        if(nextis('"')){
            strcpy(tok, "$\"");
            return '"';
        }
        strcpy(tok, "$");
        return '$';
    /*x: [[yylex()]] switch c cases */
    case '|':
        lastdol = false;
        if(nextis('|')){
            skipnl();
            strcpy(tok, "||");
            return OROR;
        }
        // FALLTHROUGH
    case '<':
    case '>':
        lastdol = false;
        /*s: [[yylex()]] in switch when redirection character */
        /*
         * funny redirection tokens:
         *	redir:	arrow | arrow '[' fd ']'
         *	arrow:	'<' | '<<' | '>' | '>>' | '|'
         *	fd:	digit | digit '=' | digit '=' digit
         *	digit:	'0'|'1'|'2'|'3'|'4'|'5'|'6'|'7'|'8'|'9'
         * some possibilities are nonsensical and get a message.
         */
        *w++=c;
        t = newtree();
        switch(c){
        /*s: [[yylex()]] in switch when redirection character, switch c cases */
        case '|':
            t->type = PIPE;
            t->fd0 = 1; // left fd of pipe (stdout of left cmd)
            t->fd1 = 0; // right fd of pipe (stdin of right cmd)
            break;
        /*x: [[yylex()]] in switch when redirection character, switch c cases */
        case '>':
            t->type = REDIR;
            if(nextis('>')){
                t->rtype = APPEND;
                *w++=c;
            }
            else 
                t->rtype = WRITE;
            t->fd0 = 1;
            break;
        /*x: [[yylex()]] in switch when redirection character, switch c cases */
        case '<':
            t->type = REDIR;
            /*s: [[yylex()]] in switch when redirection character, if here document */
            if(nextis(c)){
                t->rtype = HERE;
                *w++=c;
            }
            /*e: [[yylex()]] in switch when redirection character, if here document */
            /*s: [[yylex()]] in switch when redirection character, if read/write redirect */
            else if (nextis('>')){
                t->rtype = RDWR;
                *w++=c;
            }
            /*e: [[yylex()]] in switch when redirection character, if read/write redirect */
            else 
                t->rtype = READ;
            t->fd0 = 0;
            break;
        /*e: [[yylex()]] in switch when redirection character, switch c cases */
        }
        /*s: [[yylex()]] in switch when redirection character, if bracket after */
        if(nextis('[')){
            *w++='[';
            c = advance();
            *w++=c;
            if(c<'0' || '9'<c){
            RedirErr:
                *w = 0;
                yyerror(t->type==PIPE?"pipe syntax"
                        :"redirection syntax");
                return EOF;
            }
            t->fd0 = 0;
            do{
                t->fd0 = t->fd0*10 + c-'0';
                *w++=c;
                c = advance();
            }while('0'<=c && c<='9');

            if(c=='='){
                *w++='=';
                if(t->type==REDIR)
                    // change the token type
                    t->type = DUP;
                c = advance();
                if('0'<=c && c<='9'){
                    t->rtype = DUPFD;
                    t->fd1 = t->fd0;
                    t->fd0 = 0;
                    do{
                        t->fd0 = t->fd0*10+c-'0';
                        *w++=c;
                        c = advance();
                    }while('0'<=c && c<='9');
                }
                else{
                    if(t->type==PIPE)
                        goto RedirErr;
                    t->rtype = CLOSE;
                }
            }
            if(c!=']'
            || t->type==DUP && (t->rtype==HERE || t->rtype==APPEND))
                goto RedirErr;
            *w++=']';
        }
        /*e: [[yylex()]] in switch when redirection character, if bracket after */
        *w='\0';
        yylval.tree = t;
        if(t->type==PIPE)
            skipnl();
        return t->type;
        /*e: [[yylex()]] in switch when redirection character */
    /*x: [[yylex()]] switch c cases */
    case '\'':
        inquote = true;
        lastword = true;
        lastdol = false;
        for(;;){
            c = advance();
            if(c==EOF)
                break;
            if(c=='\''){
                if(nextc()!='\'')
                    break;
                advance();
            }
            w = addutf(w, c);
        }
        if(w != nil)
            *w='\0';

        t = token(tok, WORD);
        t->quoted = true;

        yylval.tree = t;
        return t->type;
    /*e: [[yylex()]] switch c cases */
    }
    // else
    /*s: [[yylex()]] if c is not a word character */
    if(!wordchr(c)){
        lastdol = false;
        tok[0] = c;
        tok[1]='\0';
        return c;
    }
    /*e: [[yylex()]] if c is not a word character */
    // else
    /*s: [[yylex()]] if c is a word character */
    for(;;){
        /*s: [[yylex()]] when c is a word character, if glob character */
        if(c=='*' || c=='[' || c=='?' || c==GLOB)
            w = addtok(w, GLOB);
        /*e: [[yylex()]] when c is a word character, if glob character */
        w = addutf(w, c);

        c = nextc();
        if(lastdol ? !idchr(c) : !wordchr(c)) 
            break;
        advance();
    }

    lastword = true;
    lastdol = false;
    if(w!=nil)
        *w='\0';

    t = klook(tok);
    if(t->type != WORD)
        lastword = false;

    t->quoted = false;

    yylval.tree = t;
    return t->type;
    /*e: [[yylex()]] if c is a word character */
}
/*e: function [[yylex]] */
/*e: rc/lex.c */
