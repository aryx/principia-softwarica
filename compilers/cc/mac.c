/*s: cc/mac.c */
#include	"cc.h"

//old: #include	"macbody"
//TODO copy paste with aa/, maybe could factorize with another lib?
//

void	macdef(void);
void	macinc(void);
void	macprag(void);
void	maclin(void);
void	macif(int);
void	macend(void);
void	macund(void);

long	getnsn(void);
Sym*	getsym(void);

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
    // else isalpha() or '_'
    for(cp = symb;;) {
        if(cp <= symb+NSYMB-4)
            *cp++ = c;
        c = getc();
        if(isalnum(c) || c == '_' || c >= Runeself)
            continue;
        unget(c);
        break;
    }
    *cp = '\0';
    if(cp > symb+NSYMB-4)
        yyerror("symbol too large: %s", symb);
    return lookup();
}
/*e: function getsym */

/*s: function getsymdots */
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
        // else c == '/'
        c = getc();
        /*s: [[getcom()]] if [[//]] comment */
        if(c == '/') {
            while(c != '\n')
                c = getc();
            break;
        }
        /*e: [[getcom()]] if [[//]] comment */
        if(c != '*')
            break;
        // else c == '*'
        /*s: [[getcom()]] when [[/*]] comment */
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
        /*e: [[getcom()]] when [[/*]] comment */
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

    strcpy(symb, cp);
    p = strchr(symb, '=');
    if(p) {
        *p++ = '\0';
        s = lookup();
        l = strlen(p) + 2;	/* +1 null, +1 nargs */
      
        while(l & 3)
            l++;
        while(nhunk < l)
            gethunk();
        *hunk = '\0'; // 0 nargs

        strcpy(hunk+1, p);
        s->macro = hunk;

        hunk += l;
        nhunk -= l;

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
    "ifdef",    nil,    /* macif(0) */
    "ifndef",   nil,    /* macif(1) */
    "else",     nil,    /* macif(2) */

    "line",     maclin,
    "define",   macdef,
    "include",  macinc,
    "undef",    macund,

    "pragma",   macprag,
    "endif",    macend,
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
    /*s: [[domacro()]] set s to endif symbol if no symbol */
    if(s == S)
        s = slookup("endif");
    /*e: [[domacro()]] set s to endif symbol if no symbol */

    for(i=0; mactab[i].macname; i++)
        if(strcmp(s->name, mactab[i].macname) == 0) {
            if(mactab[i].macf)
                // dispatcher!
                (*mactab[i].macf)();
            else
                macif(i);
            return;
        }
    // else
    yyerror("unknown #: %s", s->name);
    macend();
}
/*e: function domacro */

/*s: function macund */
void
macund(void)
{
    Sym *s;

    // lexing
    s = getsym();
    macend();
    if(s == S) {
        yyerror("syntax in #undef");
        return;
    }

    // action
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
    char *args[NARG]; // parameters
    // option<int> (None = -1), size of parameters
    int n;
    bool dots;
    char *base; // body
    /*s: [[macdef()]] other locals */
    Sym *s;
    Sym *a;
    char *np;
    int i, c, len;
    int ischr; // quote character
    /*e: [[macdef()]] other locals */

    // lexing 
    /*s: [[macdef]] lexing the macro name */
    s = getsym();
    if(s == S)
        goto bad;
    if(s->macro)
        yyerror("macro redefined: %s", s->name);
    /*e: [[macdef]] lexing the macro name */
    /*s: [[macdef]] lexing the parameters */
    c = getc();
    n = -1; // no parameter (which is different than 0 argument)
    dots = false;

    if(c == '(') {
        n++; // at least 0 parameters now
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
    /*e: [[macdef]] lexing the parameters */
    /*s: [[macdef]] lexing the body */
    base = hunk;
    len = 1;
    ischr = 0;

    for(;;) {
        /*s: [[macdef()]] when lexing the body, if identifier */
        // identifier
        if(isalpha(c) || c == '_') {
            np = symb;
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
            // else
            base = allocn(base, len, 2);
            base[len++] = '#';
            base[len++] = 'a' + i;
            continue;
        }
        /*e: [[macdef()]] when lexing the body, if identifier */
        /*s: [[macdef()]] when lexing the body, if string or character or comment */
        if(ischr){
            // inside a string or character
            if(c == '\\'){ 
                base = allocn(base, len, 1);
                base[len++] = c;
                c = getc();
            }else if(c == ischr)
                ischr = 0;
        }else{
            // string or character
            if(c == '"' || c == '\''){
                base = allocn(base, len, 1);
                base[len++] = c;
                ischr = c;
                c = getc();
                continue;
            }
            /*s: [[macdef()]] if comment */
            // comment
            if(c == '/') {
                c = getc();
                if(c == '/'){
                    // // comment
                    c = getc();
                    for(;;) {
                        if(c == '\n')
                            break;
                        c = getc();
                    }
                    continue;
                }
                if(c == '*'){
                    // /* comment
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
            /*e: [[macdef()]] if comment */
        }
        /*e: [[macdef()]] when lexing the body, if string or character or comment */

        // antislash outside a string
        if(c == '\\') {
            c = getc();
            if(c == '\n') {
                c = getc();
                continue;
            }
            // windows
            else if(c == '\r') {
                c = getc();
                if(c == '\n') {
                    c = getc();
                    continue;
                }
            }
            // else
            base = allocn(base, len, 1);
            base[len++] = '\\';
            continue;
        }
        if(c == '\n')
            break;

        if(c == '#') // escape # by adding an extra #
          if(n > 0) {
            base = allocn(base, len, 1);
            base[len++] = c;
        }
        base = allocn(base, len, 1);
        base[len++] = c;

        // GETC
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
    /*e: [[macdef]] lexing the body */

    // action

    *base = n+1; // nargs (+1 to differentiate #define foo() from #define foo)
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
    int  nargs;
    bool dots;
    int  c;
    /*s: [[macexpand()]] other locals */
    char buf[2000];
    int n;
    int l; // depth
    char *arg[NARG]; // the arguments
    char *cp, *ecp;
    char *ob = b;
    /*e: [[macexpand()]] other locals */

    // if macro has no parameter
    if(*s->macro == '\0') {
        strcpy(b, s->macro+1);
        /*s: [[macexpand()]] debug macro expansion */
        if(debug['m'])
            print("#expand %s %s\n", s->name, ob);
        /*e: [[macexpand()]] debug macro expansion */
        return;
    }
    // else, macro has parameters
    
    nargs = (char)(*s->macro & ~VARMAC) - 1;
    dots = *s->macro & VARMAC;

    c = getnsc();
    if(c != '(')
        goto bad;

    /*s: [[macexpand()]] parsing the arguments */
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

            /*s: [[macexpand()]] when parsing the arguments, if string */
            // string argument
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
            /*e: [[macexpand()]] when parsing the arguments, if string */
            /*s: [[macexpand()]] when parsing the arguments, if character */
            // quote argument
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
            /*e: [[macexpand()]] when parsing the arguments, if character */
            /*s: [[macexpand()]] when parsing the arguments, if comment */
            // comment
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
            /*e: [[macexpand()]] when parsing the arguments, if comment */
            if(l == 0) {
                if(c == ',') {
                    if(n == nargs && dots) {
                        *cp++ = ',';
                        continue;
                    }
                    *cp++ = '\0';
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
        *cp = '\0';
    }
    if(n != nargs) {
        yyerror("argument mismatch expanding: %s", s->name);
        *b = '\0';
        return;
    }
    /*e: [[macexpand()]] parsing the arguments */

    /*s: [[macexpand()]] substituting the arguments */
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
        // else
        c = *cp++;
        if(c == 0)
            goto bad;
        if(c == '#') { // # are escaped as double ##
            *b++ = c;
            continue;
        }
        c -= 'a';
        if(c < 0 || c >= n)
            continue;
        strcpy(b, arg[c]);
        b += strlen(arg[c]);
    }
    *b = '\0';
    /*e: [[macexpand()]] substituting the arguments */

    /*s: [[macexpand()]] debug macro expansion */
    if(debug['m'])
        print("#expand %s %s\n", s->name, ob);
    /*e: [[macexpand()]] debug macro expansion */

    return;

bad:
    yyerror("syntax in macro expansion: %s", s->name);
    *b = '\0';
    return;

toobig:
    yyerror("too much text in macro expansion: %s", s->name);
    *b = '\0';
}
/*e: function macexpand */

/*s: function macinc */
void
macinc(void)
{
    char str[STRINGSZ];
    char *hp = str;
    int i;
    /*s: [[macinc()]] other locals */
    int c0, c;
    /*e: [[macinc()]] other locals */
    fdt f;

    // lexing

    /*s: [[macinc()]] lexing the included filename */
    c0 = getnsc();
    if(c0 != '"') {
        c = c0;
        if(c0 != '<')
            goto bad;
        c0 = '>';
    }
    for(;;) {
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
    /*e: [[macinc()]] lexing the included filename */

    // action

    /*s: [[macinc()]] finding the included filename full path */
    f = -1;
    for(i=0; i<ninclude; i++) {
        if(i == 0 && c0 == '>') // do not look in '.' for system headers
            continue;

        strcpy(symb, include[i]);
        strcat(symb, "/");
        if(strcmp(symb, "./") == 0)
            symb[0] = '\0';
        strcat(symb, str);

        f = open(symb, 0);
        if(f >= 0)
            break;

    }
    if(f < 0)
        strcpy(symb, str);
    /*e: [[macinc()]] finding the included filename full path */

    c = strlen(symb) + 1; 

    while(c & 3)
        c++;
    while(nhunk < c)
        gethunk();
    hp = hunk;
    nhunk -= c;
    hunk += c;

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

    // lexing

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
    *cp = '\0';
    c = getcom();
    if(c != '\n')
        goto bad;

    // action

nn:
    c = strlen(symb) + 1;
    while(c & 3)
        c++;

    while(nhunk < c)
        gethunk();
    cp = hunk;
    memcpy(hunk, symb, c);
    nhunk -= c;
    hunk += c;

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
    Sym *s;
    /*s: [[macif()]] other locals */
    int c;
    int l; // depth
    bool bol; // beginning of line
    /*e: [[macif()]] other locals */

    if(f == 2) // else
        goto skip;

    // lexing

    /*s: [[macif()]] lexing the symbol */
    s = getsym();
    if(s == S)
        goto bad;
    if(getcom() != '\n')
        goto bad;
    /*e: [[macif()]] lexing the symbol */

    if((s->macro != nil) ^ f)
        return;
    // else

    // action
    /*s: [[macif()]] skipping text */
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
            // else a #xxx
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
    /*e: [[macif()]] skipping text */

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

    // lexing

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
    
        while(nhunk < c)
            gethunk();
        hp = hunk;
        memcpy(hunk, symb, c);
        nhunk -= c;
        hunk += c;
    
        h = alloc(sizeof(Hist));
        h->name = hp;
        h->line = lineno;
        h->offset = -1; // pragma

        // add_list(h, hist)
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
linehist(char *f, int offset)
{
    Hist *h;

    /*s: [[linehist()]] possibly overwrite last line directive */
    /*
     * overwrite the last #line directive if
     * no alloc has happened since the last one
     */
    if(!newflag && ehist != H && offset != 0 && ehist->offset != 0)
        if(f && ehist->name && strcmp(f, ehist->name) == 0) {
            ehist->line = lineno;
            ehist->offset = offset;
            return;
        }
    newflag = false;
    /*e: [[linehist()]] possibly overwrite last line directive */
    /*s: [[linehist()]] debug */
    if(debug['f'])
        if(f) {
            if(offset)
                print("%4ld: %s (#line %d)\n", lineno, f, offset);
            else
                print("%4ld: %s\n", lineno, f);
        } else
            print("%4ld: <pop>\n", lineno);
    /*e: [[linehist()]] debug */

    h = alloc(sizeof(Hist));
    h->name = f;
    h->line = lineno;
    h->offset = offset;

    // add_list(h, hist)
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

/*e: cc/mac.c */
