/*s: rc/lex.c */
#include "rc.h"
#include "exec.h"
#include "io.h"
#include "getflags.h"
#include "fns.h"
#include "x.tab.h"

int getnext(void);


/*s: function wordchr */
int
wordchr(int c)
{
    return !strchr("\n \t#;&|^$=`'{}()<>", c) && c!=EOF;
}
/*e: function wordchr */

/*s: function idchr */
int
idchr(int c)
{
    /*
     * Formerly:
     * return 'a'<=c && c<='z' || 'A'<=c && c<='Z' || '0'<=c && c<='9'
     *	|| c=='_' || c=='*';
     */
    return c>' ' && !strchr("!\"#$%&'()+,-./:;<=>?@[\\]^`{|}~", c);
}
/*e: function idchr */

/*s: global future */
int future = EOF;
/*e: global future */
/*s: global doprompt */
bool doprompt = true;
/*e: global doprompt */
/*s: global inquote */
bool inquote;
/*e: global inquote */
/*s: global incomm */
bool incomm;
/*e: global incomm */

/*s: function nextc */
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
/*e: function nextc */
/*s: function advance */
/*
 * Consume the lookahead character.
 */

int
advance(void)
{
    int c = nextc();
    lastc = future;
    future = EOF;
    return c;
}
/*e: function advance */
/*s: function getnext */
/*
 * read a character from the input stream
 */	
int
getnext(void)
{
    int c;
    static int peekc = EOF;

    if(peekc!=EOF){
        c = peekc;
        peekc = EOF;
        return c;
    }
    if(runq->eof)
        return EOF;
    if(doprompt)
        pprompt();

    c = rchr(runq->cmdfd);
    if(!inquote && c=='\\'){
        c = rchr(runq->cmdfd);
        if(c=='\n' && !incomm){		/* don't continue a comment */
            doprompt = true;
            c=' ';
        }
        else{
            peekc = c;
            c='\\';
        }
    }
    doprompt = doprompt || c=='\n' || c==EOF;
    if(c==EOF)
        runq->eof++; // ->eof = true cleaner no?
    else 
        if(flag['V'] || ndot>=2 && flag['v'])
            pchr(err, c);

    return c;
}
/*e: function getnext */

/*s: function pprompt */
void
pprompt(void)
{
    var *prompt;
    if(runq->iflag){
        pstr(err, promptstr);
        flush(err);
        prompt = vlook("prompt");
        if(prompt->val && prompt->val->next)
            promptstr = prompt->val->next->word;
        else
            promptstr="\t";
    }
    runq->lineno++;
    doprompt = false;
}
/*e: function pprompt */

/*s: function skipwhite */
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
        else return;
    }
}
/*e: function skipwhite */

/*s: function skipnl */
void
skipnl(void)
{
    int c;
    for(;;){
        skipwhite();
        c = nextc();
        if(c!='\n')
            return;
        advance();
    }
}
/*e: function skipnl */

/*s: function nextis */
bool
nextis(int c)
{
    if(nextc()==c){
        advance();
        return true;
    }
    return false;
}
/*e: function nextis */

/*s: function addtok */
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
/*e: function addtok */

/*s: function addutf */
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
/*e: function addutf */

/*s: global lastdol */
bool lastdol;	/* was the last token read '$' or '$#' or '"'? */
/*e: global lastdol */
/*s: global lastword (rc/lex.c) */
bool lastword;	/* was the last token read a word or compound word terminator? */
/*e: global lastword (rc/lex.c) */

/*s: function yylex */
//@Scheck: called from yyparse()
int yylex(void)
{
    int c;
    int d = nextc();
    char *w = tok;
    struct Tree *t;

    yylval.tree = 0;
    /*
     * Embarassing sneakiness:  if the last token read was a quoted or unquoted
     * WORD then we alter the meaning of what follows.  If the next character
     * is `(', we return SUB (a subscript paren) and consume the `('.  Otherwise,
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
        if(wordchr(d) || d=='\'' || d=='`' || d=='$' || d=='"'){ //$
            strcpy(tok, "^");
            return '^';
        }
    }
    inquote = false;
    skipwhite();

    switch(c = advance()){
    case EOF:
        lastdol = false;
        strcpy(tok, "EOF");
        return EOF;
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
        return '$'; //$
    case '&':
        lastdol = false;
        if(nextis('&')){
            skipnl();
            strcpy(tok, "&&");
            return ANDAND;
        }
        strcpy(tok, "&");
        return '&';
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
        case '|':
            t->type = PIPE;
            t->fd0 = 1;
            t->fd1 = 0;
            break;
        case '>':
            t->type = REDIR;
            if(nextis(c)){
                t->rtype = APPEND;
                *w++=c;
            }
            else t->rtype = WRITE;
            t->fd0 = 1;
            break;
        case '<':
            t->type = REDIR;
            if(nextis(c)){
                t->rtype = HERE;
                *w++=c;
            } else if (nextis('>')){
                t->rtype = RDWR;
                *w++=c;
            } else t->rtype = READ;
            t->fd0 = 0;
            break;
        }
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
                t->fd0 = t->fd0*10+c-'0';
                *w++=c;
                c = advance();
            }while('0'<=c && c<='9');
            if(c=='='){
                *w++='=';
                if(t->type==REDIR)
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
        *w='\0';
        yylval.tree = t;
        if(t->type==PIPE)
            skipnl();
        return t->type;

    case '\'':
        lastdol = false;
        lastword = true;
        inquote = true;
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
        if(w!=nil)
            *w='\0';

        t = token(tok, WORD);

        t->quoted = true;
        yylval.tree = t;
        return t->type;
    } // end switch

    if(!wordchr(c)){
        lastdol = false;
        tok[0] = c;
        tok[1]='\0';
        return c;
    }
    for(;;){
        if(c=='*' || c=='[' || c=='?' || c==GLOB)
            w = addtok(w, GLOB);
        w = addutf(w, c);
        c = nextc();
        if(lastdol ? !idchr(c) : !wordchr(c)) break;
        advance();
    }

    lastword = true;
    lastdol = false;
    if(w!=nil)
        *w='\0';
    t = klook(tok);
    if(t->type!=WORD)
        lastword = false;
    t->quoted = false;
    yylval.tree = t;
    return t->type;
}
/*e: function yylex */
/*e: rc/lex.c */
