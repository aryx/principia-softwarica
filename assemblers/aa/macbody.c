/*s: assemblers/aa/macbody.c */
#include "aa.h"

// forward decls
void	macund(void);
void	macdef(void);
void	macinc(void);
void	macprag(void);
void	maclin(void);
void	macif(int);
void	macend(void);

/*s: constant [[VARMAC]] */
#define VARMAC 0x80
/*e: constant [[VARMAC]] */

/*s: function [[getnsn]] */
long
getnsn(void)
{
    long n;
    int c;

    c = getnsc();
    if(c < '0' || c > '9')
        return -1;
    n = 0;
    while(c >= '0' && c <= '9') {
        n = n*10 + c-'0';
        c = getc();
    }
    unget(c);
    return n;
}
/*e: function [[getnsn]] */

/*s: function [[getsym]] */
Sym*
getsym(void)
{
    int c;
    char *cp;

    c = getnsc();
    if(!isalpha(c) && c != '_' && c < Runeself) {
        unget(c);
        return S;
    }
    for(cp = symb;;) {
        if(cp <= symb+NSYMB-4)
            *cp++ = c;
        c = getc();
        if(isalnum(c) || c == '_' || c >= Runeself)
            continue;
        // else
        unget(c);
        break;
    }
    *cp = '\0';
    /*s: [[getsym()]] sanity check cp */
    if(cp > symb+NSYMB-4)
        yyerror("symbol too large: %s", symb);
    /*e: [[getsym()]] sanity check cp */
    return lookup();
}
/*e: function [[getsym]] */

/*s: function [[getsymdots]] */
Sym*
getsymdots(bool *dots)
{
    int c;
    Sym *s;

    s = getsym();
    if(s != S)
        return s;

    c = getnsc();
    if(c != '.'){
        unget(c);
        return S;
    }
    if(getc() != '.' || getc() != '.')
        yyerror("bad dots in macro");
    *dots = true;
    return slookup("__VA_ARGS__");
}
/*e: function [[getsymdots]] */

/*s: function [[getcom]] */
int
getcom(void)
{
    int c;

    for(;;) {
        c = getnsc(); // skip whitespaces

        if(c != '/')
            break;
        c = getc();
        if(c == '/') {
            while(c != '\n')
                c = getc();
            break;
        }
        if(c != '*')
            break;
        c = getc();
        for(;;) {
            if(c == '*') {
                c = getc();
                if(c != '/')
                    continue;
                c = getc();
                break;
            }
            if(c == '\n') {
                yyerror("comment across newline");
                break;
            }
            c = getc();
        }
        if(c == '\n')
            break;
    }
    return c;
}
/*e: function [[getcom]] */

/*s: function [[dodefine]] */
void
dodefine(char *cp)
{
    Sym *s;
    char *p;
    long l;
    char *x;

    strcpy(symb, cp);
    p = strchr(symb, '=');
    if(p) {
        *p++ = '\0';
        s = lookup();
        l = strlen(p) + 2;	/* +1 null, +1 nargs */

        while(l & 3)
            l++;
        x = malloc(l);

        *x = '\0';
        strcpy(x+1, p);
        s->macro = x;
    } else {
        s = lookup();
        s->macro = "\0001";	/* \000 is nargs */
    }
    /*s: [[dodefine()]] debug */
    if(debug['m'])
        print("#define (-D) %s %s\n", s->name, s->macro+1);
    /*e: [[dodefine()]] debug */
}
/*e: function [[dodefine]] */

/*s: global [[mactab]] */
struct
{
    char    *macname;
    void    (*macf)(void);
} mactab[] =
{
    "ifdef",    nil,    /* macif(0) */
    "ifndef",   nil,    /* macif(1) */
    "else",     nil,    /* macif(2) */
    "endif",    macend,

    "include",  macinc,
    "line",     maclin,
    "define",   macdef,
    "undef",    macund,

    "pragma",   macprag,
    0
};
/*e: global [[mactab]] */

/*s: function [[domacro]] */
/// main -> assemble -> yyparse -> yylex -> <>
void
domacro(void)
{
    int i;
    Sym *s;

    s = getsym();
    /*s: [[domacro()]] set s to endif symbol if no symbol */
    if(s == S)
        s = slookup("endif");
    /*e: [[domacro()]] set s to endif symbol if no symbol */

    for(i=0; mactab[i].macname; i++)
        if(strcmp(s->name, mactab[i].macname) == 0) {
            // dispatch!
            if(mactab[i].macf)
                (*mactab[i].macf)();
            else
                macif(i);
            return;
        }
    // else
    /*s: [[domacro()]] handle unknown directives */
    yyerror("unknown #: %s", s->name);
    macend();
    /*e: [[domacro()]] handle unknown directives */
}
/*e: function [[domacro]] */

/*s: function [[macund]] */
void
macund(void)
{
    Sym *s;

    s = getsym();
    macend();
    if(s == S) {
        yyerror("syntax in #undef");
        return;
    }
    s->macro = nil;
}
/*e: function [[macund]] */

/*s: constant [[NARG]] */
#define	NARG	25
/*e: constant [[NARG]] */
/*s: function [[macdef]] */
void
macdef(void)
{
    Sym *s, *a;
    char *args[NARG], *np, *base;
    int n, i, c, len;
    bool dots;
    int ischr;

    s = getsym();
    if(s == S)
        goto bad;
    if(s->macro)
        yyerror("macro redefined: %s", s->name);
    c = getc();
    n = -1;
    dots = 0;
    if(c == '(') {
        n++;
        c = getnsc();
        if(c != ')') {
            unget(c);
            for(;;) {
                a = getsymdots(&dots);
                if(a == S)
                    goto bad;
                if(n >= NARG) {
                    yyerror("too many arguments in #define: %s", s->name);
                    goto bad;
                }
                args[n++] = a->name;
                c = getnsc();
                if(c == ')')
                    break;
                if(c != ',' || dots)
                    goto bad;
            }
        }
        c = getc();
    }
    if(isspace(c))
        if(c != '\n')
            c = getnsc();

    base = hunk;

    len = 1;
    ischr = 0;
    for(;;) {
        if(isalpha(c) || c == '_') {
            np = symb; // jarod: overflow?
            *np++ = c;
            c = getc();
            while(isalnum(c) || c == '_') {
                *np++ = c;
                c = getc();
            }
            *np = '\0';
            for(i=0; i<n; i++)
                if(strcmp(symb, args[i]) == 0)
                    break;
            if(i >= n) {
                i = strlen(symb);
                base = allocn(base, len, i);
                memcpy(base+len, symb, i);
                len += i;
                continue;
            }
            base = allocn(base, len, 2);
            base[len++] = '#';
            base[len++] = 'a' + i;
            continue;
        }
        if(ischr){
            if(c == '\\'){ 
                base = allocn(base, len, 1);
                base[len++] = c;
                c = getc();
            }else if(c == ischr)
                ischr = 0;
        }else{
            if(c == '"' || c == '\''){
                base = allocn(base, len, 1);
                base[len++] = c;
                ischr = c;
                c = getc();
                continue;
            }
            if(c == '/') {
                c = getc();
                if(c == '/'){
                    c = getc();
                    for(;;) {
                        if(c == '\n')
                            break;
                        c = getc();
                    }
                    continue;
                }
                if(c == '*'){
                    c = getc();
                    for(;;) {
                        if(c == '*') {
                            c = getc();
                            if(c != '/')
                                continue;
                            c = getc();
                            break;
                        }
                        if(c == '\n') {
                            yyerror("comment and newline in define: %s", s->name);
                            break;
                        }
                        c = getc();
                    }
                    continue;
                }
                base = allocn(base, len, 1);
                base[len++] = '/';
                continue;
            }
        }
        if(c == '\\') {
            c = getc();
            if(c == '\n') {
                c = getc();
                continue;
            }
            else if(c == '\r') {
                c = getc();
                if(c == '\n') {
                    c = getc();
                    continue;
                }
            }
            base = allocn(base, len, 1);
            base[len++] = '\\';
            continue;
        }
        if(c == '\n')
            break;
        if(c == '#')
        if(n > 0) {
            base = allocn(base, len, 1);
            base[len++] = c;
        }
        base = allocn(base, len, 1);
        base[len++] = c;
        c = ((--fi.c < 0)? filbuf(): (*fi.p++ & 0xff)); //jarod: GETC
        if(c == '\n')
            lineno++;
        if(c == -1) { // jarod: EOF
            yyerror("eof in a macro: %s", s->name);
            break;
        }
    }
    do {
        base = allocn(base, len, 1);
        base[len++] = 0;
    } while(len & 3);

    *base = n+1;
    if(dots)
        *base |= VARMAC;
    s->macro = base;
    /*s: [[macdef()]] debug */
    if(debug['m'])
        print("#define %s %s\n", s->name, s->macro+1);
    /*e: [[macdef()]] debug */
    return;

bad:
    if(s == S)
        yyerror("syntax in #define");
    else
        yyerror("syntax in #define: %s", s->name);
    macend();
}
/*e: function [[macdef]] */

/*s: function [[macexpand]] */
void
macexpand(Sym *s, char *b)
{
    char buf[2000];
    int n, l, c, nargs;
    char *arg[NARG], *cp, *ob, *ecp, dots;

    ob = b;
    if(*s->macro == 0) {
        strcpy(b, s->macro+1);
        /*s: [[macexpand()]] debug part1 */
        if(debug['m'])
            print("#expand %s %s\n", s->name, ob);
        /*e: [[macexpand()]] debug part1 */
        return;
    }
    
    nargs = (char)(*s->macro & ~VARMAC) - 1;
    dots = *s->macro & VARMAC;

    c = getnsc();
    if(c != '(')
        goto bad;
    n = 0;
    c = getc();
    if(c != ')') {
        unget(c);
        l = 0;
        cp = buf;
        ecp = cp + sizeof(buf)-4;
        arg[n++] = cp;
        for(;;) {
            if(cp >= ecp)
                goto toobig;
            c = getc();
            if(c == '"')
                for(;;) {
                    if(cp >= ecp)
                        goto toobig;
                    *cp++ = c;
                    c = getc();
                    if(c == '\\') {
                        *cp++ = c;
                        c = getc();
                        continue;
                    }
                    if(c == '\n') //jarod: how can have this in a macro def?
                        goto bad;
                    if(c == '"')
                        break;
                }
            if(c == '\'')
                for(;;) {
                    if(cp >= ecp)
                        goto toobig;
                    *cp++ = c;
                    c = getc();
                    if(c == '\\') {
                        *cp++ = c;
                        c = getc();
                        continue;
                    }
                    if(c == '\n')
                        goto bad;
                    if(c == '\'')
                        break;
                }
            if(c == '/') {
                c = getc();
                switch(c) {
                case '*':
                    for(;;) {
                        c = getc();
                        if(c == '*') {
                            c = getc();
                            if(c == '/')
                                break;
                        }
                    }
                    *cp++ = ' ';
                    continue;
                case '/':
                    while((c = getc()) != '\n')
                        ;
                    break;
                default:
                    unget(c);
                    c = '/';
                }
            }
            if(l == 0) {
                if(c == ',') {
                    if(n == nargs && dots) {
                        *cp++ = ',';
                        continue;
                    }
                    *cp++ = 0;
                    arg[n++] = cp;
                    if(n > nargs)
                        break;
                    continue;
                }
                if(c == ')')
                    break;
            }
            if(c == '\n')
                c = ' ';
            *cp++ = c;
            if(c == '(')
                l++;
            if(c == ')')
                l--;
        }
        *cp = 0;
    }
    if(n != nargs) {
        yyerror("argument mismatch expanding: %s", s->name);
        *b = 0;
        return;
    }
    cp = s->macro+1;
    for(;;) {
        c = *cp++;
        if(c == '\n')
            c = ' ';
        if(c != '#') {
            *b++ = c;
            if(c == 0)
                break;
            continue;
        }
        c = *cp++;
        if(c == 0)
            goto bad;
        if(c == '#') {
            *b++ = c;
            continue;
        }
        c -= 'a';
        if(c < 0 || c >= n)
            continue;
        strcpy(b, arg[c]);
        b += strlen(arg[c]);
    }
    *b = 0;
    /*s: [[macexpand()]] debug part2 */
    if(debug['m'])
        print("#expand %s %s\n", s->name, ob);
    /*e: [[macexpand()]] debug part2 */
    return;

bad:
    yyerror("syntax in macro expansion: %s", s->name);
    *b = 0;
    return;

toobig:
    yyerror("too much text in macro expansion: %s", s->name);
    *b = 0;
}
/*e: function [[macexpand]] */

/*s: function [[macinc]] */
void
macinc(void)
{
    char *hp;
    int n;
    fdt f = -1;
    /*s: [[macinc()]] other locals */
    char str[STRINGSZ];
    int c, cend;
    /*x: [[macinc()]] other locals */
    int i;
    /*e: [[macinc()]] other locals */

    // lexing

    /*s: [[macinc()]] lexing the included filename */
    cend = getnsc();
    if(cend != '"') {
        if(cend != '<') {
            c = cend;
            goto bad;
        }
        cend = '>';
    }
    // cend = '"' or '>'
    hp = str;
    for(;;) {
        c = getc();
        if(c == cend)
            break;
        if(c == '\n')
            goto bad;
        *hp++ = c;
    }
    *hp = '\0';

    /*s: [[macinc()]] finish parsing the line */
    c = getcom();
    if(c != '\n')
        goto bad;
    /*e: [[macinc()]] finish parsing the line */
    /*e: [[macinc()]] lexing the included filename */

    // action

    /*s: [[macinc()]] find and store the included filename full path in [[symb]] */
    for(i=0; i<ninclude; i++) {
        /*s: [[macinc()]] skipped first entry for system headers */
        if(i == 0 && cend == '>') // do not look in '.' for system headers
            continue;
        /*e: [[macinc()]] skipped first entry for system headers */
        strcpy(symb, include[i]);
        strcat(symb, "/");
        /*s: [[macinc()]] normalize path */
        if(strcmp(symb, "./") == 0)
            symb[0] = '\0';
        /*e: [[macinc()]] normalize path */
        strcat(symb, str);

        f = open(symb, OREAD);
        if(f >= 0)
            break;
        // else, try another directory
    }
    // could not find a directory, maybe it was an absolute path
    if(f < 0)
        strcpy(symb, str);
    /*e: [[macinc()]] find and store the included filename full path in [[symb]] */

    n = strlen(symb) + 1;

    while(n & 3)
        n++;
    hp = malloc(n);
    memcpy(hp, symb, n);

    newio();
    pushio();
    newfile(hp, f);

    return;

/*s: [[macinc()]] bad */
bad:
    unget(c);
    yyerror("syntax in #include");
    macend();
/*e: [[macinc()]] bad */
}
/*e: function [[macinc]] */

/*s: function [[maclin]] */
void
maclin(void)
{
    char *cp;
    long n;
    int size;
    /*s: [[maclin()]] other locals */
    int c;
    /*e: [[maclin()]] other locals */

    // lexing

    /*s: [[maclin()]] lexing the line [[n]] and filename in [[symb]] */
    // the line number

    n = getnsn();

    // the (optional) filename

    c = getc();
    if(n < 0)
        goto bad;

    for(;;) {
        /*s: [[maclin()]] skipping whitespaces */
        if(c == ' ' || c == '\t') {
            c = getc();
            continue;
        }
        /*e: [[maclin()]] skipping whitespaces */
        if(c == '"')
            break;
        /*s: [[maclin()]] if no filename */
        if(c == '\n') {
            strcpy(symb, "<noname>");
            goto nn;
        }
        /*e: [[maclin()]] if no filename */
        // else
        goto bad;
    }
    cp = symb;
    for(;;) {
        c = getc();
        if(c == '"')
            break;
        *cp++ = c;
    }
    *cp = '\0';

    /*s: [[maclin()]] finish parsing the line */
    c = getcom();
    if(c != '\n')
        goto bad;
    /*e: [[maclin()]] finish parsing the line */
    /*e: [[maclin()]] lexing the line [[n]] and filename in [[symb]] */

    // action
nn:
    size = strlen(symb) + 1;

    while(size & 3)
        size++;
    cp = malloc(size);

    memcpy(cp, symb, size);

    /*s: [[maclin()]] call linehist */
    linehist(cp, n);
    /*e: [[maclin()]] call linehist */
    return;

/*s: [[maclin()]] bad */
bad:
    unget(c);
    yyerror("syntax in #line");
    macend();
/*e: [[maclin()]] bad */
}
/*e: function [[maclin]] */

/*s: function [[macif]] */
void
macif(int f)
{
    int c, l;
    bool bol;
    Sym *s;

    if(f == 2)
        goto skip;
    s = getsym();
    if(s == S)
        goto bad;
    if(getcom() != '\n')
        goto bad;
    if((s->macro != nil) ^ f)
        return;

skip:
    bol = true;
    l = 0;
    for(;;) {
        c = getc();
        if(c != '#') {
            if(!isspace(c))
                bol = false;
            if(c == '\n')
                bol = true;
            continue;
        }
        if(!bol)
            continue;
        s = getsym();
        if(s == S)
            continue;
        if(strcmp(s->name, "endif") == 0) {
            if(l) {
                l--;
                continue;
            }
            macend();
            return;
        }
        if(strcmp(s->name, "ifdef") == 0 || strcmp(s->name, "ifndef") == 0) {
            l++;
            continue;
        }
        if(l == 0 && f != 2 && strcmp(s->name, "else") == 0) {
            macend();
            return;
        }
    }

bad:
    yyerror("syntax in #if(n)def");
    macend();
}
/*e: function [[macif]] */

/*s: function [[macprag]] */
void
macprag(void)
{
    Sym *s;
    /*s: [[macprag()]] locals */
    Hist *h;
    char *hp;
    int c0, c;
    /*e: [[macprag()]] locals */

    s = getsym();

    /*s: [[macprag()]] if pragma lib */
    if(s && strcmp(s->name, "lib") == 0) {
        c0 = getnsc();
        if(c0 != '"') {
            c = c0;
            if(c0 != '<')
                goto bad;
            c0 = '>';
        }
        for(hp = symb;;) {
            c = getc();
            if(c == c0)
                break;
            if(c == '\n')
                goto bad;
            *hp++ = c;
        }
        *hp = '\0';
        c = getcom();
        if(c != '\n')
            goto bad;

        /*
         * put pragma-line in as a funny history 
         */
        c = strlen(symb) + 1;
        while(c & 3)
            c++;

        hp = malloc(c);
        memcpy(hp, symb, c);

        h = alloc(sizeof(Hist));
        h->filename = hp;
        h->global_line = lineno;
        h->local_line = -1; // ugly, special mark for #pragma lib in Hist

        h->link = H;
        if(ehist == H) {
            hist = h;
            ehist = h;
            return;
        }
        ehist->link = h;
        ehist = h;

        return;

    bad:
        unget(c);
        yyerror("syntax in #pragma lib");
        macend();

    }
    /*e: [[macprag()]] if pragma lib */
    else {
        while(getnsc() != '\n')
            ;
        return;
    }
}
/*e: function [[macprag]] */

/*s: function [[macend]] */
void
macend(void)
{
    int c;

    for(;;) {
        c = getnsc();
        if(c < 0 || c == '\n')
            return;
    }
}
/*e: function [[macend]] */

/*e: assemblers/aa/macbody.c */
