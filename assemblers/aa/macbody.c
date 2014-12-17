/*s: assemblers/aa/macbody.c */
#include "aa.h"

void	macund(void);
void	macdef(void);
void	macinc(void);
void	macprag(void);
void	maclin(void);
void	macif(int);
void	macend(void);

/*s: constant VARMAC */
#define VARMAC 0x80
/*e: constant VARMAC */

/*s: function getnsn */
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
/*e: function getnsn */

/*s: function getsym */
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
        unget(c);
        break;
    }
    *cp = 0;
    if(cp > symb+NSYMB-4)
        yyerror("symbol too large: %s", symb);
    return lookup();
}
/*e: function getsym */

/*s: function getsymdots */
Sym*
getsymdots(int *dots)
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
    *dots = 1;
    return slookup("__VA_ARGS__");
}
/*e: function getsymdots */

/*s: function getcom */
int
getcom(void)
{
    int c;

    for(;;) {
        c = getnsc();
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
/*e: function getcom */

/*s: function dodefine */
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
        *p++ = 0;
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
/*e: function dodefine */

/*s: global mactab */
struct
{
    char	*macname;
    void	(*macf)(void);
} mactab[] =
{
    "ifdef",	nil,	/* macif(0) */
    "ifndef",	nil,	/* macif(1) */
    "else",		nil,	/* macif(2) */
    "endif",	macend,

    "include",	macinc,
    "define",	macdef,
    "undef",	macund,
    "pragma",	macprag,
    "line",		maclin,
    0
};
/*e: global mactab */

/*s: function domacro */
void
domacro(void)
{
    int i;
    Sym *s;

    s = getsym();
    if(s == S)
        s = slookup("endif");

    for(i=0; mactab[i].macname; i++)
        if(strcmp(s->name, mactab[i].macname) == 0) {
            if(mactab[i].macf)
                // dispatcher!
                (*mactab[i].macf)();
            else
                macif(i);
            return;
        }
    yyerror("unknown #: %s", s->name);
    macend();
}
/*e: function domacro */

/*s: function macund */
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
/*e: function macund */

/*s: constant NARG */
#define	NARG	25
/*e: constant NARG */
/*s: function macdef */
void
macdef(void)
{
    Sym *s, *a;
    char *args[NARG], *np, *base;
    int n, i, c, len, dots;
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
            np = symb;
            *np++ = c;
            c = getc();
            while(isalnum(c) || c == '_') {
                *np++ = c;
                c = getc();
            }
            *np = 0;
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
        c = ((--fi.c < 0)? filbuf(): (*fi.p++ & 0xff));
        if(c == '\n')
            lineno++;
        if(c == -1) {
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
/*e: function macdef */

/*s: function macexpand */
void
macexpand(Sym *s, char *b)
{
    char buf[2000];
    int n, l, c, nargs;
    char *arg[NARG], *cp, *ob, *ecp, dots;

    ob = b;
    if(*s->macro == 0) {
        strcpy(b, s->macro+1);
        if(debug['m'])
            print("#expand %s %s\n", s->name, ob);
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
                    if(c == '\n')
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
    if(debug['m'])
        print("#expand %s %s\n", s->name, ob);
    return;

bad:
    yyerror("syntax in macro expansion: %s", s->name);
    *b = 0;
    return;

toobig:
    yyerror("too much text in macro expansion: %s", s->name);
    *b = 0;
}
/*e: function macexpand */

/*s: function macinc */
void
macinc(void)
{
    int c0, c, i, f;
    char str[STRINGSZ], *hp;

    c0 = getnsc();
    if(c0 != '"') {
        c = c0;
        if(c0 != '<')
            goto bad;
        c0 = '>';
    }
    for(hp = str;;) {
        c = getc();
        if(c == c0)
            break;
        if(c == '\n')
            goto bad;
        *hp++ = c;
    }
    *hp = 0;

    c = getcom();
    if(c != '\n')
        goto bad;

    f = -1;
    for(i=0; i<ninclude; i++) {
        if(i == 0 && c0 == '>')
            continue;
        strcpy(symb, include[i]);
        strcat(symb, "/");
        if(strcmp(symb, "./") == 0)
            symb[0] = 0;
        strcat(symb, str);

        f = open(symb, 0);
        if(f >= 0)
            break;

    }
    if(f < 0)
        strcpy(symb, str);
    c = strlen(symb) + 1;
    while(c & 3)
        c++;

    hp = malloc(c);
    memcpy(hp, symb, c);

    newio();
    pushio();
    newfile(hp, f);
    return;

bad:
    unget(c);
    yyerror("syntax in #include");
    macend();
}
/*e: function macinc */

/*s: function maclin */
void
maclin(void)
{
    char *cp;
    int c;
    long n;

    n = getnsn();
    c = getc();
    if(n < 0)
        goto bad;

    for(;;) {
        if(c == ' ' || c == '\t') {
            c = getc();
            continue;
        }
        if(c == '"')
            break;
        if(c == '\n') {
            strcpy(symb, "<noname>");
            goto nn;
        }
        goto bad;
    }
    cp = symb;
    for(;;) {
        c = getc();
        if(c == '"')
            break;
        *cp++ = c;
    }
    *cp = 0;
    c = getcom();
    if(c != '\n')
        goto bad;

nn:
    c = strlen(symb) + 1;
    while(c & 3)
        c++;

    cp = malloc(c);
    memcpy(cp, symb, c);

    linehist(cp, n);
    return;

bad:
    unget(c);
    yyerror("syntax in #line");
    macend();
}
/*e: function maclin */

/*s: function macif */
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
/*e: function macif */

/*s: function macprag */
void
macprag(void)
{
    Sym *s;
    int c0, c;
    char *hp;
    Hist *h;

    s = getsym();

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
        h->line = lineno;
        h->local_line = -1;

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

    } else {
        while(getnsc() != '\n')
            ;
        return;
    }
}
/*e: function macprag */

/*s: function macend */
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
/*e: function macend */

/*s: function linehist */
void
linehist(char *f, int local_line)
{
    Hist *h;

    /*s: [[linehist()]] debug */
    if(debug['f'])
        if(f) {
            if(local_line)
                print("%4ld: %s (#line %d)\n", lineno, f, local_line);
            else
                print("%4ld: %s\n", lineno, f);
        } else
            print("%4ld: <pop>\n", lineno);
    /*e: [[linehist()]] debug */

    h = alloc(sizeof(Hist));
    h->filename = f;
    h->line = lineno;
    h->local_line = local_line;

    //add_list(hist, ehist, h)
    h->link = H;
    if(ehist == H) {
        hist = h;
        ehist = h;
        return;
    }
    ehist->link = h;
    ehist = h;
}
/*e: function linehist */

/*s: function gethunk */
void
gethunk(void)
{
    char *h;
    long nh;

    nh = NHUNK;
    if(thunk >= 10L*NHUNK)
        nh = 10L*NHUNK;

    h = (char*)sbrk(nh);
    if(h == (char*)-1) {
        yyerror("out of memory");
        errorexit();
    }
    hunk = h;
    nhunk = nh;
    thunk += nh;
}
/*e: function gethunk */
/*e: assemblers/aa/macbody.c */
