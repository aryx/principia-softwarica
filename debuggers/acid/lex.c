/*s: acid/lex.c */
#include <u.h>
#include <libc.h>

#include <bio.h>
#include <ctype.h>
#include <mach.h>

#include "acid.h"
#include "y.tab.h"

/*s: global [[keywds]] */
struct keywd
{
    char	*name;
    int	terminal;
}
keywds[] =
{
    "do",	Tdo,
    "if",	Tif,
    "then",	Tthen,
    "else",	Telse,
    "while",	Twhile,
    "loop",	Tloop,

    "head",	Thead,
    "tail",	Ttail,
    "append",	Tappend,

    "defn",	Tfn,
    "return",	Tret,
    "local",	Tlocal,
    "aggr",	Tcomplex,
    "union",	Tcomplex,
    "adt",	Tcomplex,
    "complex",	Tcomplex,
    "delete",	Tdelete,
    "whatis",	Twhat,
    "eval",	Teval,
    "builtin",	Tbuiltin,

    0,		0
};
/*e: global [[keywds]] */

/*s: global [[cmap]] */
char cmap[256] =
{
    ['0']	'\0'+1,
    ['n']	'\n'+1,
    ['r']	'\r'+1,
    ['t']	'\t'+1,
    ['b']	'\b'+1,
    ['f']	'\f'+1,
    ['a']	'\a'+1,
    ['v']	'\v'+1,
    ['\\']	'\\'+1,
    ['"']	'"'+1,
};
/*e: global [[cmap]] */

/*s: function [[kinit]] */
void
kinit(void)
{
    int i;
    
    for(i = 0; keywds[i].name; i++) 
        enter(keywds[i].name, keywds[i].terminal);
}
/*e: function [[kinit]] */

typedef struct IOstack IOstack;
/*s: struct [[IOstack]] */
struct IOstack
{
    // ref_own<string>    included filename or "<stdin>"  or "<string>"
    char	*name;
    // option<ref_own<Biobuf>> (None when <string>)
    Biobuf	*fin;

    // saved global line to be restored in popio()
    int	line;

    /*s: [[IOstack]] other fields */
    char	*text;
    char	*ip;
    /*e: [[IOstack]] other fields */
    // Extra
    /*s: [[IOstack]] extra fields */
    IOstack	*prev;
    /*e: [[IOstack]] extra fields */
};
/*e: struct [[IOstack]] */
/*s: global [[lexio]] */
// list<ref_own<IOstack> next = lexio.prev
IOstack *lexio;
/*e: global [[lexio]] */

/*s: function [[pushfile]] */
/// main | loadmodule -> <>
void
pushfile(char *file)
{
    Biobuf *b;
    IOstack *io;

    if(file)
        b = Bopen(file, OREAD);
    else{
        b = Bopen("/fd/0", OREAD);
        file = "<stdin>";
    }

    /*s: [[pushfile()]] sanity check [[b]] */
    if(b == nil)
        error("pushfile: %s: %r", file);
    /*e: [[pushfile()]] sanity check [[b]] */

    io = malloc(sizeof(IOstack));
    /*s: [[pushfile()]] sanity check [[io]] */
    if(io == nil)
        fatal("no memory");
    /*e: [[pushfile()]] sanity check [[io]] */
    io->name = strdup(file);
    /*s: [[pushfile()]] sanity check [[io->name]] */
    if(io->name == nil)
        fatal("no memory");
    /*e: [[pushfile()]] sanity check [[io->name]] */

    io->line = line;
    line = 1;
    io->fin = b;
    io->text = nil;

    // add_list(io, lexio)
    io->prev = lexio;
    lexio = io;
}
/*e: function [[pushfile]] */
/*s: function [[pushstr]] */
/// acid: interpret() -> interpret -> <>
void
pushstr(Node *s)
{
    IOstack *io;

    io = malloc(sizeof(IOstack));
    /*s: [[pushstr()]] sanity check [[io]] */
    if(io == nil)
        fatal("no memory");
    /*e: [[pushstr()]] sanity check [[io]] */
    io->line = line;
    line = 1;
    io->name = strdup("<string>");
    /*s: [[pushstr()]] sanity check [[io->name]] */
    if(io->name == nil)
        fatal("no memory");
    /*e: [[pushstr()]] sanity check [[io->name]] */
    io->line = line;
    line = 1;

    // this time we do not use io->fin but io->text
    io->fin = nil;
    io->text = strdup(s->string->string);
    /*s: [[pushstr()]] sanity check [[io->text]] */
    if(io->text == 0)
        fatal("no memory");
    /*e: [[pushstr()]] sanity check [[io->text]] */
    io->ip = io->text;

    io->prev = lexio;
    lexio = io;
}
/*e: function [[pushstr]] */

/*s: function [[restartio]] */
/// main -> yyparse; <>
void
restartio(void)
{
    Bflush(lexio->fin);
    Binit(lexio->fin, STDIN, OREAD);
}
/*e: function [[restartio]] */
/*s: function [[popio]] */
/// loadmodule | error -> <>
bool
popio(void)
{
    IOstack *s;

    if(lexio == nil)
        return false;

    if(lexio->prev == nil){
        if(lexio->fin)
            restartio();
        return false;
    }
    // else

    if(lexio->fin)
        Bterm(lexio->fin);
    else
       /*s: [[popio()]] when no [[fin]] */
       free(lexio->text);
       /*e: [[popio()]] when no [[fin]] */
    free(lexio->name);

    // restore global line
    line = lexio->line;

    // s = pop_list(lexio)
    s = lexio;
    lexio = s->prev;
    free(s);

    return true;
}
/*e: function [[popio]] */

/*s: function [[Lfmt]] */
int
Lfmt(Fmt *f)
{
    int i;
    char buf[1024];
    IOstack *e;

    e = lexio;
    if(e) {
        i = snprint(buf, sizeof(buf), "%s:%d", e->name, line);
        while(e->prev) {
            e = e->prev;
            if(initialising && e->prev == 0)
                break;
            i += snprint(buf+i, sizeof(buf)-i, " [%s:%d]", e->name, e->line);
        }
    } else
        snprint(buf, sizeof(buf),  "no file:0");
    fmtstrcpy(f, buf);
    return 0;
}
/*e: function [[Lfmt]] */

/*s: function [[unlexc]] */
void
unlexc(int s)
{
    if(s == '\n')
        line--;

    if(lexio->fin)
        Bungetc(lexio->fin);
    else
        /*s: [[unlexc()]] when no [[fin]] */
        lexio->ip--;
        /*e: [[unlexc()]] when no [[fin]] */
}
/*e: function [[unlexc]] */
/*s: function [[lexc]] */
int
lexc(void)
{
    int c;

    if(lexio->fin) {
        c = Bgetc(lexio->fin);
        /*s: [[lexc()]] if [[gotint]] */
        if(gotint)
            error("interrupt");
        /*e: [[lexc()]] if [[gotint]] */
        return c;
    }
    // else
    /*s: [[lexc()]] when no [[fin]] */
    c = *lexio->ip++;
    if(c == 0)
        return -1;
    return c;
    /*e: [[lexc()]] when no [[fin]] */
}
/*e: function [[lexc]] */

/*s: function [[escchar]] */
int
escchar(char c)
{
    int n;
    char buf[Strsize];

    if(c >= '0' && c <= '9') {
        n = 1;
        buf[0] = c;
        for(;;) {
            c = lexc();
            if(c == Eof)
                error("%d: <eof> in escape sequence", line);
            if(strchr("0123456789xX", c) == 0) {
                unlexc(c);
                break;
            }
            if(n >= Strsize)
                error("string escape too long");
            buf[n++] = c;
        }
        buf[n] = '\0';
        return strtol(buf, 0, 0);
    }

    n = cmap[c];
    if(n == 0)
        return c;
    return n-1;
}
/*e: function [[escchar]] */

/*s: function [[eatstring]] */
void
eatstring(void)
{
    int esc, c, cnt;
    char buf[Strsize];

    esc = 0;
    for(cnt = 0;;) {
        c = lexc();
        switch(c) {
        case Eof:
            error("%d: <eof> in string constant", line);

        case '\n':
            error("newline in string constant");
            goto done;

        case '\\':
            if(esc)
                goto Default;
            esc = 1;
            break;

        case '"':
            if(esc == 0)
                goto done;

            /* Fall through */
        default:
        Default:
            if(esc) {
                c = escchar(c);
                esc = 0;
            }
            buf[cnt++] = c;
            break;
        }
        if(cnt >= Strsize)
            error("string token too long");
    }
done:
    buf[cnt] = '\0';
    yylval.string = strnode(buf);
}
/*e: function [[eatstring]] */

/*s: function [[eatnl]] */
void
eatnl(void)
{
    int c;

    line++;
    for(;;) {
        c = lexc();
        if(c == Eof)
            error("eof in comment");
        if(c == '\n')
            return;
    }
}
/*e: function [[eatnl]] */

/*s: function [[yylex]] */
/// main -> yyparse -> <>
int
yylex(void)
{
    int c;
    /*s: [[yylex]] other locals */
    extern char vfmt[];
    /*e: [[yylex]] other locals */

loop:
    Bflush(bout);
    c = lexc();
    switch(c) {
    /*s: [[yylex()]] switch [[c]] cases */
    case Eof:
        /*s: [[yylex()]] when [[Eof]], if [[gotint]] */
        if(gotint) {
            gotint = false;
            stacked = 0;
            Bprint(bout, "\nacid: ");
            goto loop;
        }
        /*e: [[yylex()]] when [[Eof]], if [[gotint]] */
        return Eof;
    /*x: [[yylex()]] switch [[c]] cases */
    case '.':
        c = lexc();
        unlexc(c);
        if(isdigit(c))
            return numsym('.');

        return '.';

    /*x: [[yylex()]] switch [[c]] cases */
    default:
        return numsym(c);
    /*x: [[yylex()]] switch [[c]] cases */
    case ' ':
    case '\t':
        goto loop;
    /*x: [[yylex()]] switch [[c]] cases */
    case '/':
        c = lexc();
        //pad: the '*' case is just to support syncweb comments
        if(c == '/' || c == '*') {
            eatnl();
            goto loop;
        }
        unlexc(c);
        return '/';

    /*x: [[yylex()]] switch [[c]] cases */
    case '\n':
        line++;
        if(!interactive)
            goto loop;
        if(stacked) {
            print("\t");
            goto loop;
        }
        // else when interactive and not stacked
        return ';';
    /*x: [[yylex()]] switch [[c]] cases */
    case '\'':
        c = lexc();
        if(c == '\\')
            yylval.ival = escchar(lexc());
        else
            yylval.ival = c;
        c = lexc();
        if(c != '\'') {
            error("missing '");
            unlexc(c);
        }
        return Tconst;

    /*x: [[yylex()]] switch [[c]] cases */
    case '"':
        eatstring();
        return Tstring;
    /*x: [[yylex()]] switch [[c]] cases */
    case '(':
    case ')':
    case '[':
    case ']':
    case ';':
    case ':':
    case ',':
    case '~':
    case '?':
    case '*':
    case '@':
    case '^':
    case '%':
        return c;
    /*x: [[yylex()]] switch [[c]] cases */
    case '{':
        stacked++;
        return c;
    case '}':
        stacked--;
        return c;

    /*x: [[yylex()]] switch [[c]] cases */
    case '!':
        c = lexc();
        if(c == '=')
            return Tneq;
        unlexc(c);
        return '!';

    /*x: [[yylex()]] switch [[c]] cases */
    case '+':
        c = lexc();
        if(c == '+')
            return Tinc;
        unlexc(c);
        return '+';

    /*x: [[yylex()]] switch [[c]] cases */
    case '&':
        c = lexc();
        if(c == '&')
            return Tandand;
        unlexc(c);
        return '&';

    /*x: [[yylex()]] switch [[c]] cases */
    case '=':
        c = lexc();
        if(c == '=')
            return Teq;
        unlexc(c);
        return '=';

    /*x: [[yylex()]] switch [[c]] cases */
    case '|':
        c = lexc();
        if(c == '|')
            return Toror;
        unlexc(c);
        return '|';

    /*x: [[yylex()]] switch [[c]] cases */
    case '<':
        c = lexc();
        if(c == '=')
            return Tleq;
        if(c == '<')
            return Tlsh;
        unlexc(c);
        return '<';

    case '>':
        c = lexc();
        if(c == '=')
            return Tgeq;
        if(c == '>')
            return Trsh;
        unlexc(c);
        return '>';

    /*x: [[yylex()]] switch [[c]] cases */
    case '-':
        c = lexc();

        if(c == '>')
            return Tindir;

        if(c == '-')
            return Tdec;
        unlexc(c);
        return '-';

    /*x: [[yylex()]] switch [[c]] cases */
    case '\\':
        c = lexc();
        if(strchr(vfmt, c) == 0) {
            unlexc(c);
            return '\\';
        }
        yylval.ival = c;
        return Tfmt;

    /*e: [[yylex()]] switch [[c]] cases */
    }
}
/*e: function [[yylex]] */

/*s: function [[numsym]] */
int
numsym(char first)
{
    char *p;
    int c;
    /*s: [[numsym()]] locals */
    bool isbin, isfloat, ishex;
    char *sel;
    /*x: [[numsym()]] locals */
    Lsym *s;
    /*e: [[numsym()]] locals */

    symbol[0] = first;
    p = symbol;

    /*s: [[numsym()]] if number */
    ishex = false;
    isbin = false;
    isfloat = false;
    if(first == '.')
        isfloat = true;

    if(isdigit(*p++) || isfloat) {
        for(;;) {
            c = lexc();
            if(c < 0)
                error("%d: <eof> eating symbols", line);

            if(c == '\n')
                line++;
            sel = "01234567890.xb";
            if(ishex)
                sel = "01234567890abcdefABCDEF";
            else if(isbin)
                sel = "01";
            else if(isfloat)
                sel = "01234567890eE-+";

            if(strchr(sel, c) == 0) {
                unlexc(c);
                break;
            }
            if(c == '.')
                isfloat = true;
            if(!isbin && c == 'x')
                ishex = true;
            if(!ishex && c == 'b')
                isbin = true;
            *p++ = c;
        }
        *p = '\0';
        if(isfloat) {
            yylval.fval = atof(symbol);
            return Tfconst;
        }

        if(isbin)
            yylval.ival = strtoull(symbol+2, 0, 2);
        else
            yylval.ival = strtoull(symbol, 0, 0);
        return Tconst;
    }
    /*e: [[numsym()]] if number */
    // else
    /*s: [[numsym()]] when symbol */
    for(;;) {
        c = lexc();
        if(c < 0)
            error("%d <eof> eating symbols", line);
        if(c == '\n')
            line++;
        if(c != '_' && c != '$' && c <= '~' && !isalnum(c)) {	/* checking against ~ lets UTF names through */
            unlexc(c);
            break;
        }
        *p++ = c;
    }

    *p = '\0';

    s = look(symbol);
    if(s == 0)
        s = enter(symbol, Tid);

    yylval.sym = s;
    return s->lexval;
    /*e: [[numsym()]] when symbol */
}
/*e: function [[numsym]] */

/*s: function [[enter]] */
Lsym*
enter(char *name, int t)
{
    Lsym *s;
    uint h;
    char *p;
    /*s: [[enter()]] other locals */
    Value *v;
    /*e: [[enter()]] other locals */

    // dupe of look()?
    h = 0;
    for(p = name; *p; p++)
        h = h*3 + *p;
    h %= Hashsize;

    s = gmalloc(sizeof(Lsym));
    memset(s, 0, sizeof(Lsym));
    s->name = strdup(name);
    s->lexval = t;

    s->hash = hash[h];
    hash[h] = s;

    /*s: [[enter()]] allocate value of symbol */
    v = gmalloc(sizeof(Value));
    s->v = v;

    v->fmt = 'X';
    v->type = TINT;
    memset(v, 0, sizeof(Value));
    /*e: [[enter()]] allocate value of symbol */

    return s;
}
/*e: function [[enter]] */

/*s: function [[look]] */
Lsym*
look(char *name)
{
    Lsym *s;
    uint h;
    char *p;

    h = 0;
    for(p = name; *p; p++)
        h = h*3 + *p;
    h %= Hashsize;

    for(s = hash[h]; s; s = s->hash)
        if(strcmp(name, s->name) == 0)
            return s;
    // else
    return nil;
}
/*e: function [[look]] */
/*s: function [[mkvar]] */
Lsym*
mkvar(char *s)
{
    Lsym *l;

    l = look(s);
    if(l == nil)
        l = enter(s, Tid);
    return l;
}
/*e: function [[mkvar]] */
/*e: acid/lex.c */
