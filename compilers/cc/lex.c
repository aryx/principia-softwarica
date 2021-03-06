/*s: cc/lex.c */
#include	"cc.h"
#include	"y.tab.h"

int	compile(char*, char**, int);
void	syminit(Sym*);
int	mpatov(char*, vlong*);
long	getr(void);
long	escchar(long, int, int);
void	cinit(void);

int	Oconv(Fmt*);
int	Lconv(Fmt*);
int	Tconv(Fmt*);
int	FNconv(Fmt*);
int	Qconv(Fmt*);
int	VBconv(Fmt*);

void	setinclude(char*);


#ifndef	CPP
/*s: constant [[CPP]] */
#define	CPP	"/bin/cpp"
/*e: constant [[CPP]] */
#endif

/*
 * known debug flags
 *	-a		acid declaration output
 *	-A		!B
 *	-B		non ANSI
 *	-d		print declarations
 *	-D name		define
 *	-F		format specification check
 *	-i		print initialization
 *	-I path		include
 *	-l		generate little-endian code
 *	-L		print every NAME symbol
 *	-M		constant multiplication
 *	-m		print add/sub/mul trees
 *	-n		print acid to file (%.c=%.acid) (with -a or -aa)
 *	-o file		output file
 *	-p		use standard cpp ANSI preprocessor (not on windows)
 *	-r		print registerization
 *	-s		print structure offsets (with -a or -aa)
 *	-S		print assembly
 *	-t		print type trees
 *	-V		enable void* conversion warnings
 *	-v		verbose printing
 *	-w		print warnings
 *	-X		abort on error
 *	-.		Inhibit search for includes in source directory
 */

/*s: function [[main]] */
//@Scheck: not dead, entry point :)
void main(int argc, char *argv[])
{
    int err;
    /*s: [[main()]] locals */
    // growing_array<string>
    char **defs = nil;
    int ndef = 0;
    /*x: [[main()]] locals */
    char *p;
    /*x: [[main()]] locals */
    char **np;
    int maxdef = 0;
    /*x: [[main()]] locals */
    int c;
    /*x: [[main()]] locals */
    int nproc, nout, status, i;
    /*e: [[main()]] locals */

    memset(debug, 0, sizeof(debug));
    /*s: [[main()]] xxxinit() */
    tinit(); // type globals initialisation
    cinit(); // C lexing/parsing globals initialisation
    ginit(); // arch dependent globals initialisation, 5c/8c/...
    arginit(); // printf argument checking initialisation
    /*e: [[main()]] xxxinit() */
    /*s: [[main()]] remaining initialisation */
    setinclude(".");
    /*x: [[main()]] remaining initialisation */
    profileflg = true;	/* #pragma can turn it off */
    /*x: [[main()]] remaining initialisation */
    tufield = simplet((1L<<tfield->etype) | BUNSIGNED);
    /*e: [[main()]] remaining initialisation */

    ARGBEGIN {
    /*s: [[main()]] command line processing */
    case 'o':
        outfile = ARGF();
        break;
    /*x: [[main()]] command line processing */
    case 'D':
        p = ARGF();
        if(p) {
           /*s: [[main()]] when -D in command line processing, grow defs array */
            // realloc, growing array
            if(ndef >= maxdef){
                maxdef += 50;
                np = alloc(maxdef * sizeof *np);
                if(defs != nil)
                    memmove(np, defs, (maxdef - 50) * sizeof *np);
                defs = np;
            }
           /*e: [[main()]] when -D in command line processing, grow defs array */
            defs[ndef++] = p;

            dodefine(p);
        }
        break;
    /*x: [[main()]] command line processing */
    case 'I':
        p = ARGF();
        if(p)
            setinclude(p);
        break;
    /*x: [[main()]] command line processing */
    default:
        c = ARGC();
        if(c >= 0 && c < sizeof(debug))
            debug[c]++;
        break;
    /*e: [[main()]] command line processing */
    } ARGEND

    if(argc < 1 && outfile == nil) {
        print("usage: %cc [-options] files\n", thechar);
        errorexit();
    }

    /*s: [[main()]] multiple files handling */
    if(argc > 1) {
        nproc = 1;
        /*
         * if we're writing acid to standard output, don't compile
         * concurrently, to avoid interleaving output.
         */
        if(((!debug['a'] && !debug['Z']) || debug['n']) &&
            (p = getenv("NPROC")) != nil)
            nproc = atol(p);	/* */
        c = 0;
        nout = 0;
        for(;;) {
            while(nout < nproc && argc > 0) {
                i = fork();
                if(i < 0) {
                    i = mywait(&status);
                    if(i < 0) {
                        print("cannot create a process\n");
                        errorexit();
                    }
                    if(status)
                        c++;
                    nout--;
                    continue;
                }
                if(i == 0) {
                    fprint(2, "%s:\n", *argv);
                    if (compile(*argv, defs, ndef))
                        errorexit();
                    exits(0);
                }
                nout++;
                argc--;
                argv++;
            }
            i = mywait(&status);
            if(i < 0) {
                if(c)
                    errorexit();
                exits(0);
            }
            if(status)
                c++;
            nout--;
        }
    }
    /*e: [[main()]] multiple files handling */
    //else

    err = compile((argc == 0) ? "stdin" : argv[0], defs, ndef);

    if(err)
        errorexit();
    exits(nil);
}
/*e: function [[main]] */

/*s: function [[compile]] */
int
compile(char *infile, char **defs, int ndef)
{
    /*s: [[compile()]] locals */
    char *p;
    fdt ofd;
    static bool first = true;
    /*x: [[compile()]] locals */
    char ofile[400];
    /*x: [[compile()]] locals */
    char incfile[20];
    /*x: [[compile()]] locals */
    char **av, opt[256];
    int i, c, fd[2];
    /*e: [[compile()]] locals */

    /*s: [[compile()]] set p to basename(infile) and adjust include */
    strcpy(ofile, infile);
    p = utfrrune(ofile, pathchar());
    if(p) {
        *p++ = '\0';
        if(!debug['.'])
            include[0] = strdup(ofile);
    } else
        p = ofile;
    /*e: [[compile()]] set p to basename(infile) and adjust include */

    if(outfile == nil) {
        /*s: [[compile()]] set outfile, using p, to {basename(infile)}.{thechar} */
        outfile = p;
        if(outfile) {
            if(p = utfrrune(outfile, '.'))
                if(p[1] == 'c' && p[2] == 0)
                    p[0] = '\0';
            p = utfrune(outfile, 0);
            /*s: [[compile()]] adjust p for outfile if acid option */
            if(debug['a'] && debug['n'])
                strcat(p, ".acid");
            /*e: [[compile()]] adjust p for outfile if acid option */
            /*s: [[compile()]] adjust p for outfile if pickle option */
            else if(debug['Z'] && debug['n'])
                strcat(p, "_pickle.c");
            /*e: [[compile()]] adjust p for outfile if pickle option */
            else {
                p[0] = '.';
                p[1] = thechar;
                p[2] = '\0';
            }
        } else
            outfile = "/dev/null";
        /*e: [[compile()]] set outfile, using p, to {basename(infile)}.{thechar} */
    }

    /*s: [[compile()]] setinclude("/{thestring}/include") or INCLUDE */
    if(p = getenv("INCLUDE")) {
        setinclude(p);
    } else {
        if(systemtype(Plan9)) {
            sprint(incfile, "/%s/include", thestring);
            setinclude(strdup(incfile));
            setinclude("/sys/include");
        }
    }
    /*e: [[compile()]] setinclude("/{thestring}/include") or INCLUDE */

    if (first)
        Binit(&diagbuf, STDOUT, OWRITE);
    first = false;

    /*s: [[compile()]] if writing acid to standard output */
    /*
     * if we're writing acid to standard output, don't keep scratching
     * outbuf.
     */
    if((debug['a'] || debug['Z']) && !debug['n']) {
        if (first) {
            outfile = nil;
            Binit(&outbuf, dup(1, -1), OWRITE);
            dup(2, 1);
        }
    } 
    /*e: [[compile()]] if writing acid to standard output */
    else {
        ofd = mycreat(outfile, 0664);
        /*s: [[compile()]] sanity check ofd */
        if(ofd < 0) {
            diag(Z, "cannot open %s - %r", outfile);
            outfile = nil;
            errorexit();
        }
        /*e: [[compile()]] sanity check ofd */
        Binit(&outbuf, ofd, OWRITE);
    }

    /*s: [[compile()]] initialize IO */
    // for embedded cpp
    newio();
    /*e: [[compile()]] initialize IO */

    /*s: [[compile()]] if use ANSI preprocessor */
    /* Use an ANSI preprocessor */
    if(debug['p']) {
        if(myaccess(infile) < 0) {
            diag(Z, "%s does not exist", infile);
            errorexit();
        }
        if(pipe(fd) < 0) {
            diag(Z, "pipe failed");
            errorexit();
        }
        switch(fork()) {
        case -1:
            diag(Z, "fork failed");
            errorexit();

        case 0:
            close(fd[0]);
            dup(fd[1], 1);
            close(fd[1]);
            av = alloc((3 + ndef + ninclude + 2) * sizeof *av);
            av[0] = CPP;
            i = 1;
            if(debug['.'])
                av[i++] = strdup("-.");
            /* 1999 ANSI C requires recognising // comments */
            av[i++] = strdup("-+");
            for(c = 0; c < ndef; c++) {
                sprint(opt, "-D%s", defs[c]);
                av[i++] = strdup(opt);
            }
            for(c = 0; c < ninclude; c++) {
                sprint(opt, "-I%s", include[c]);
                av[i++] = strdup(opt);
            }
            if(strcmp(infile, "stdin") != 0)
                av[i++] = infile;
            av[i] = 0;
            if(debug['p'] > 1) {
                for(c = 0; c < i; c++)
                    fprint(2, "%s ", av[c]);
                fprint(2, "\n");
            }

            exec(av[0], av);
            fprint(2, "can't exec C preprocessor %s: %r\n", CPP);
            errorexit();

        default:
            close(fd[1]);
            newfile(infile, fd[0]);
            break;
        }
    }
    /*e: [[compile()]] if use ANSI preprocessor */
    else {
        if(strcmp(infile, "stdin") == 0)
            newfile(infile, STDIN);
        else
            newfile(infile, -1);
    }
 
    // The big call!
    yyparse();

    if(!debug['a'] && !debug['Z'])
        // Another important call, calls outcode()
        gclean();
    return nerrors;
}
/*e: function [[compile]] */


/*s: function [[pushio]] */
void
pushio(void)
{
    Io *i;

    i = iostack;
    if(i == I) {
        yyerror("botch in pushio");
        errorexit();
    }
    i->p = fi.p;
    i->c = fi.c;
}
/*e: function [[pushio]] */

/*s: function [[newio]] */
void
newio(void)
{
    Io *i;

    /*s: [[newio()]] allocate a new Io in [[i]] or find a free one */
    static int pushdepth = 0;

    i = iofree;
    if(i == I) {
        /*s: [[newio()]] sanity check depth of macro expansion */
        pushdepth++;
        if(pushdepth > 1000) {
            yyerror("macro/io expansion too deep");
            errorexit();
        }
        /*e: [[newio()]] sanity check depth of macro expansion */
        i = alloc(sizeof(*i));
    } else
        // pop(iofree)
        iofree = i->link;
    /*e: [[newio()]] allocate a new Io in [[i]] or find a free one */
    i->c = 0;
    i->f = -1;
    ionext = i;
}
/*e: function [[newio]] */

/*s: function [[newfile]] */
void
newfile(char *s, int f)
{
    Io *i;

    /*s: [[newfile()]] debug */
    if(debug['e'])
        print("%L: %s\n", lineno, s);
    /*e: [[newfile()]] debug */

    // add_list(ionext, iostack)
    i = ionext;
    i->link = iostack;
    iostack = i;

    i->f = f;
    if(f < 0)
        i->f = open(s, 0);
    if(i->f < 0) {
        yyerror("%cc: %r: %s", thechar, s);
        errorexit();
    }
    fi.c = 0;

    /*s: [[newfile()]] call linehist */
    linehist(s, 0);
    /*e: [[newfile()]] call linehist */
}
/*e: function [[newfile]] */


/*s: function [[slookup]] */
Sym*
slookup(char *s)
{

    strcpy(symb, s);
    return lookup();
}
/*e: function [[slookup]] */

/*s: function [[lookup]] */
Sym*
lookup(void)
{
    Sym *s;
    ulong h;
    char *p;
    int c, n;

    // h = hash(symb)
    h = 0;
    for(p=symb; *p;) {
        h = h * 3;
        h += *p++;
    }
    n = (p - symb) + 1;
    if((long)h < 0)
        h = ~h;
    h %= NHASH;

    c = symb[0];
    for(s = hash[h]; s != S; s = s->link) {
        if(s->name[0] != c)
            continue;
        if(strcmp(s->name, symb) == 0)
            return s;
    }

    // else
    s = alloc(sizeof(Sym));
    s->name = alloc(n);
    memmove(s->name, symb, n);

    strcpy(s->name, symb);

    s->link = hash[h];
    hash[h] = s;

    syminit(s);

    return s;
}
/*e: function [[lookup]] */

/*s: function [[syminit]] */
void
syminit(Sym *s)
{
    s->lexical = LNAME;
    s->block = 0;
    s->offset = 0;

    s->type = T;
    s->class = CXXX;
    s->suetag = T;

    s->sig = SIGNONE;
    /*s: [[syminit()]] remaining initialisations */
    s->aused = false;
    /*e: [[syminit()]] remaining initialisations */
}
/*e: function [[syminit]] */

/*s: constant [[EOF]] */
#define	EOF	(-1)
/*e: constant [[EOF]] */
/*s: constant [[IGN]] */
#define	IGN	(-2)
/*e: constant [[IGN]] */
/*s: constant [[ESC]] */
#define	ESC	(Runemask+1)		/* Rune flag: a literal byte */
/*e: constant [[ESC]] */
/*s: function [[GETC]] */
#define	GETC()	((--fi.c < 0)? filbuf(): (*fi.p++ & 0xff))
/*e: function [[GETC]] */

/*s: enum [[numxxx]] */
enum Numxxx
{
    Numdec		= 1<<0,
    /*s: [[Numxxx]] cases */
    Numuns		= 1<<2,
    /*x: [[Numxxx]] cases */
    Numlong		= 1<<1,
    Numvlong	= 1<<3,
    /*e: [[Numxxx]] cases */
    Numflt		= 1<<4,
};
/*e: enum [[numxxx]] */

/*s: function [[yylex]] */
//@Scheck: not dead, called by yyparse
long yylex(void)
{
    long c; // Rune?
    /*s: [[yylex()]] locals */
    long c1;
    /*x: [[yylex()]] locals */
    char *cp;
    /*x: [[yylex()]] locals */
    Sym *s;
    /*x: [[yylex()]] locals */
    vlong vv;
    // enum<Type_kind>
    long t;
    /*x: [[yylex()]] locals */
    Rune rune;
    /*e: [[yylex()]] locals */

    /*s: [[yylex()]] peekc handling, starting part */
    if(peekc != IGN) {
        c = peekc;
        peekc = IGN;
        goto l1;
    }
    /*e: [[yylex()]] peekc handling, starting part */
l0:
    c = GETC();

l1:
    /*s: [[yylex()]] if unicode character */
    if(c >= Runeself) {
        /*
         * extension --
         *	all multibyte runes are alpha
         */
        cp = symb;
        goto talph;
    }
    /*e: [[yylex()]] if unicode character */
    if(isspace(c)) {
        /*s: [[yylex()]] if c is newline */
        if(c == '\n')
            lineno++;
        /*e: [[yylex()]] if c is newline */
        // ignore spaces
        goto l0;
    }
    /*s: [[yylex()]] before switch, if isxxx */
    if(isalpha(c)) {
        cp = symb;
        /*s: [[yylex()]] if L followed by unicode strings or character */
        if(c != 'L')
            goto talph;

        *cp++ = c;
        c = GETC();
        if(c == '\'') {
            /* L'x' */
            c = escchar('\'', true, false);
            if(c == EOF)
                c = '\'';
            c1 = escchar('\'', true, false);
            if(c1 != EOF) {
                yyerror("missing '");
                peekc = c1;
            }
            yylval.vval = convvtox(c, TRUNE);
            return LUCONST;
        }
        if(c == '"') {
            goto caselq;
        }
        /*e: [[yylex()]] if L followed by unicode strings or character */
        goto talph;
    }
    /*x: [[yylex()]] before switch, if isxxx */
    if(isdigit(c))
        goto tnum;
    /*e: [[yylex()]] before switch, if isxxx */
    switch(c) {
    /*s: [[yylex()]] switch c cases */
    case EOF:
        peekc = EOF;
        return -1; // EOF
    /*x: [[yylex()]] switch c cases */
    case '/':
        c1 = GETC();
        if(c1 == '*') {
            // '/''*' read; skip everything until next '*''/'
            for(;;) {
                c = getr();
                while(c == '*') {
                    c = getr();
                    if(c == '/')
                        goto l0;
                }
                if(c == EOF) {
                    yyerror("eof in comment");
                    errorexit();
                }
            }
        }
        if(c1 == '/') {
            // '/''/' read; skip everything until next '\n'
            for(;;) {
                c = getr();
                if(c == '\n')
                    goto l0;
                if(c == EOF) {
                    yyerror("eof in comment");
                    errorexit();
                }
            }
        }
        if(c1 == '=')
            return LDVE;
        break;
    /*x: [[yylex()]] switch c cases */
    case '_':
        cp = symb;
        goto talph;
    /*x: [[yylex()]] switch c cases */
    case '*':
        c1 = GETC();
        if(c1 == '=')
            return LMLE;
        break;
    /*x: [[yylex()]] switch c cases */
    case '%':
        c1 = GETC();
        if(c1 == '=')
            return LMDE;
        break;
    /*x: [[yylex()]] switch c cases */
    case '+':
        c1 = GETC();
        if(c1 == '+')
            return LPP;
        if(c1 == '=')
            return LPE;
        break;
    /*x: [[yylex()]] switch c cases */
    case '-':
        c1 = GETC();
        if(c1 == '-')
            return LMM;
        if(c1 == '=')
            return LME;
        if(c1 == '>')
            return LMG;
        break;
    /*x: [[yylex()]] switch c cases */
    case '>':
        c1 = GETC();
        if(c1 == '>') {
            c = LRSH;
            c1 = GETC();
            if(c1 == '=')
                return LRSHE;
            break;
        }
        if(c1 == '=')
            return LGE;
        break;
    /*x: [[yylex()]] switch c cases */
    case '<':
        c1 = GETC();
        if(c1 == '<') {
            c = LLSH;
            c1 = GETC();
            if(c1 == '=')
                return LLSHE;
            break;
        }
        if(c1 == '=')
            return LLE;
        break;
    /*x: [[yylex()]] switch c cases */
    case '=':
        c1 = GETC();
        if(c1 == '=')
            return LEQ;
        break;
    /*x: [[yylex()]] switch c cases */
    case '!':
        c1 = GETC();
        if(c1 == '=')
            return LNE;
        break;
    /*x: [[yylex()]] switch c cases */
    case '&':
        c1 = GETC();
        if(c1 == '&')
            return LANDAND;
        if(c1 == '=')
            return LANDE;
        break;
    /*x: [[yylex()]] switch c cases */
    case '|':
        c1 = GETC();
        if(c1 == '|')
            return LOROR;
        if(c1 == '=')
            return LORE;
        break;
    /*x: [[yylex()]] switch c cases */
    case '^':
        c1 = GETC();
        if(c1 == '=')
            return LXORE;
        break;
    /*x: [[yylex()]] switch c cases */
    case '.':
        c1 = GETC();
        if(isdigit(c1)) {
            cp = symb;
            *cp++ = c;
            c = c1;
            c1 = 0;
            goto casedot;
        }
        break;
    /*x: [[yylex()]] switch c cases */
    case '\'':
        /* '.' */
        c = escchar('\'', false, false);
        if(c == EOF)
            c = '\'';
        c1 = escchar('\'', false, false);
        if(c1 != EOF) {
            yyerror("missing '");
            peekc = c1;
        }

        vv = c;
        yylval.vval = convvtox(vv, TUCHAR);
        if(yylval.vval != vv)
            yyerror("overflow in character constant: 0x%lx", c);
        else
        if(c & 0x80){
            nearln = lineno;
            warn(Z, "sign-extended character constant");
        }
        yylval.vval = convvtox(vv, TCHAR);
        return LCONST;
    /*x: [[yylex()]] switch c cases */
    case '"':
        strcpy(symb, "\"<string>\""); // for error reporting
        cp = alloc(0);
        c1 = 0;

        /* "..." */
        for(;;) {
            c = escchar('"', false, true);
            if(c == EOF)
                break;
            if(c & ESC) {
                cp = allocn(cp, c1, 1);
                cp[c1++] = c;
            } else {
                rune = c;
                c = runelen(rune);
                cp = allocn(cp, c1, c);
                runetochar(cp+c1, &rune);
                c1 += c;
            }
        }
        yylval.sval.l = c1;
        do {
            cp = allocn(cp, c1, 1);
            cp[c1++] = '\0';
        } while(c1 & MAXALIGN);
        yylval.sval.s = cp;
        return LSTRING;
    /*x: [[yylex()]] switch c cases */
    caselq:
        /* L"..." */
        strcpy(symb, "\"L<string>\"");
        cp = alloc(0);
        c1 = 0;
        for(;;) {
            c = escchar('"', true, false);
            if(c == EOF)
                break;
            cp = allocn(cp, c1, sizeof(TRune));
            *(TRune*)(cp + c1) = c;
            c1 += sizeof(TRune);
        }
        yylval.sval.l = c1;
        do {
            cp = allocn(cp, c1, sizeof(TRune));
            *(TRune*)(cp + c1) = 0;
            c1 += sizeof(TRune);
        } while(c1 & MAXALIGN);
        yylval.sval.s = cp;
        return LLSTRING;
    /*x: [[yylex()]] switch c cases */
    case '#':
        domacro();
        goto l0;
    /*e: [[yylex()]] switch c cases */
    default:
        return c;
    }
    /*s: [[yylex()]] peekc handling, ending part */
    peekc = c1;
    /*e: [[yylex()]] peekc handling, ending part */
    return c;

/*s: [[yylex()]] labels */
talph:
    /*
     * cp is set to symb and some
     * prefix has been stored
     */
    for(;;) {
        /*s: [[yylex()]] in talph case, if unicode character */
        if(c >= Runeself) {
            for(c1=0;;) {
                cp[c1++] = c;
                if(fullrune(cp, c1))
                    break;
                c = GETC();
            }
            cp += c1;
            c = GETC();
            continue;
        }
        /*e: [[yylex()]] in talph case, if unicode character */
        if(!isalnum(c) && c != '_')
            break;
        *cp++ = c;
        c = GETC();
    }
    *cp = '\0';
    /*s: [[yylex()]] in talph case, debug symbol */
    if(debug['L'])
        print("%L: %s\n", lineno, symb);
    /*e: [[yylex()]] in talph case, debug symbol */
    peekc = c;

    // the important call!
    s = lookup();

    /*s: [[yylex()]] in talph case, if macro symbol */
    if(s->macro) {
        newio();
        cp = ionext->b;
        macexpand(s, cp);

        pushio();
        // push_list(ionext, iostack)
        ionext->link = iostack;
        iostack = ionext;

        fi.p = cp;
        fi.c = strlen(cp);

        /*s: [[yylex()]] in talph case, when macro symbol, peekc handling */
        if(peekc != IGN) {
            cp[fi.c++] = peekc;
            cp[fi.c] = '\0';
            peekc = IGN;
        }
        /*e: [[yylex()]] in talph case, when macro symbol, peekc handling */
        goto l0;
    }
    /*e: [[yylex()]] in talph case, if macro symbol */

    yylval.sym = s;
    /*s: [[yylex()]] alpha case, return LTYPE if typedef symbol s */
    if(s->class == CTYPEDEF || s->class == CTYPESTR)
        return LTYPE;
    /*e: [[yylex()]] alpha case, return LTYPE if typedef symbol s */
    return s->lexical;
/*x: [[yylex()]] labels */
tnum:
    c1 = 0;
    cp = symb;
    if(c != '0') {
        // not an hexa or octal number
        c1 |= Numdec;
        for(;;) {
            *cp++ = c;
            c = GETC();
            if(isdigit(c))
                continue;
            goto dc;
        }
    }
    /*s: [[yylex()]] in number case, 0xxx handling */
    // else, octal or hexadecimal number
    *cp++ = c;
    c = GETC();
    if(c == 'x' || c == 'X')
        for(;;) {
            *cp++ = c;
            c = GETC();
            if(isdigit(c))
                continue;
            if(c >= 'a' && c <= 'f')
                continue;
            if(c >= 'A' && c <= 'F')
                continue;
            if(cp == symb+2)
                yyerror("malformed hex constant");
            goto ncu;
        }
    if(c < '0' || c > '7')
        goto dc;
    for(;;) {
        if(c >= '0' && c <= '7') {
            *cp++ = c;
            c = GETC();
            continue;
        }
        goto ncu;
    }
    //Fallthrough
    /*e: [[yylex()]] in number case, 0xxx handling */
/*s: [[yylex()]] in number case, decimal dc label handling */
dc:
    /*s: [[yylex()]] in number case, in decimal case, float handling */
    if(c == '.')
        goto casedot;
    if(c == 'e' || c == 'E')
        goto casee;
    /*e: [[yylex()]] in number case, in decimal case, float handling */
    //Fallthrough
/*s: [[yylex()]] in number case, in decimal case, ncu suffix label handling */
ncu:
    if((c == 'U' || c == 'u') && !(c1 & Numuns)) {
        c = GETC();
        c1 |= Numuns;
        goto ncu;
    }
/*e: [[yylex()]] in number case, in decimal case, ncu suffix label handling */
/*s: [[yylex()]] in number case, in decimal case, long suffix label handling */
if((c == 'L' || c == 'l') && !(c1 & Numvlong)) {
    c = GETC();
    if(c1 & Numlong)
        c1 |= Numvlong;
    c1 |= Numlong;
    goto ncu;
}
/*e: [[yylex()]] in number case, in decimal case, long suffix label handling */
    *cp = '\0';
    peekc = c;

    if(mpatov(symb, &yylval.vval))
        yyerror("overflow in constant");

    vv = yylval.vval;
    /*s: [[yylex()]] in number case, in decimal case, if vlong number */
    if(c1 & Numvlong) {
        if((c1 & Numuns) || convvtox(vv, TVLONG) < 0) {
            c = LUVLCONST;
            t = TUVLONG;
            goto nret;
        }
        c = LVLCONST;
        t = TVLONG;
        goto nret;
    }
    /*e: [[yylex()]] in number case, in decimal case, if vlong number */
    /*s: [[yylex()]] in number case, in decimal case, if long number */
    if(c1 & Numlong) {
        if((c1 & Numuns) || convvtox(vv, TLONG) < 0) {
            c = LULCONST;
            t = TULONG;
            goto nret;
        }
        c = LLCONST;
        t = TLONG;
        goto nret;
    }
    /*e: [[yylex()]] in number case, in decimal case, if long number */
    /*s: [[yylex()]] in number case, in decimal case, if unsigned number */
    if((c1 & Numuns) || convvtox(vv, TINT) < 0) {
        c = LUCONST;
        t = TUINT;
        goto nret;
    }
    /*e: [[yylex()]] in number case, in decimal case, if unsigned number */
    c = LCONST;
    t = TINT;
    goto nret;
/*e: [[yylex()]] in number case, decimal dc label handling */
/*x: [[yylex()]] labels */
nret:
    yylval.vval = convvtox(vv, t);
    if(yylval.vval != vv){
        nearln = lineno;
        warn(Z, "truncated constant: %T %s", types[t], symb);
    }
    return c;
/*x: [[yylex()]] labels */
casedot:
    for(;;) {
        *cp++ = c;
        c = GETC();
        if(!isdigit(c))
            break;
    }
    if(c != 'e' && c != 'E')
        goto caseout;
    // Fallthrough

casee:
    *cp++ = 'e';
    c = GETC();
    if(c == '+' || c == '-') {
        *cp++ = c;
        c = GETC();
    }
    if(!isdigit(c))
        yyerror("malformed fp constant exponent");
    while(isdigit(c)) {
        *cp++ = c;
        c = GETC();
    }
    // Fallthrough
caseout:
    if(c == 'L' || c == 'l') {
        c = GETC();
        c1 |= Numlong; // DEAD? used?
    } else
    if(c == 'F' || c == 'f') {
        c = GETC();
        c1 |= Numflt;
    }
    *cp = 0;
    peekc = c;
    yylval.dval = strtod(symb, nil);
    if(isInf(yylval.dval, 1) || isInf(yylval.dval, -1)) {
        yyerror("overflow in float constant");
        yylval.dval = 0;
    }
    if(c1 & Numflt)
        return LFCONST;
    // else
    return LDCONST;
/*e: [[yylex()]] labels */
}
/*e: function [[yylex]] */


/*s: function [[mpatov]] */
/*
 * convert a string, s, to vlong in *v
 * return conversion overflow.
 * required syntax is [0[x]]d*
 */
bool
mpatov(char *s, vlong *v)
{
    vlong n, nn;
    int c;

    n = 0;
    c = *s;
    if(c == '0')
        goto oct;
    while(c = *s++) {
        if(c >= '0' && c <= '9')
            nn = n*10 + c-'0';
        else
            goto bad;
        if(n < 0 && nn >= 0)
            goto bad;
        n = nn;
    }
    goto out;

oct:
    s++;
    c = *s;
    if(c == 'x' || c == 'X')
        goto hex;
    while(c = *s++) {
        if(c >= '0' || c <= '7')
            nn = n*8 + c-'0';
        else
            goto bad;
        if(n < 0 && nn >= 0)
            goto bad;
        n = nn;
    }
    goto out;

hex:
    s++;
    while(c = *s++) {
        if(c >= '0' && c <= '9')
            c += 0-'0';
        else
        if(c >= 'a' && c <= 'f')
            c += 10-'a';
        else
        if(c >= 'A' && c <= 'F')
            c += 10-'A';
        else
            goto bad;
        nn = n*16 + c;
        if(n < 0 && nn >= 0)
            goto bad;
        n = nn;
    }
out:
    *v = n;
    return false;

bad:
    *v = ~0;
    return true;
}
/*e: function [[mpatov]] */

/*s: function [[getc]] */
int
getc(void)
{
    int c;

    /*s: [[getc()]] peekc handling */
    if(peekc != IGN) {
        c = peekc;
        peekc = IGN;
    }
    /*e: [[getc()]] peekc handling */
    else
        c = GETC();

    if(c == '\n')
        lineno++;
    if(c == EOF) {
        yyerror("End of file");
        errorexit();
    }
    return c;
}
/*e: function [[getc]] */

/*s: function [[getr]] */
long
getr(void)
{
    int c, i;
    char str[UTFmax+1];
    Rune rune;


    c = getc();
    if(c < Runeself)
        return c;
    i = 0;
    str[i++] = c;

loop:
    c = getc();
    str[i++] = c;
    if(!fullrune(str, i))
        goto loop;
    c = chartorune(&rune, str);
    if(rune == Runeerror && c == 1) {
        nearln = lineno;
        diag(Z, "illegal rune in string");
        for(c=0; c<i; c++)
            print(" %.2x", *(uchar*)(str+c));
        print("\n");
    }
    return rune;
}
/*e: function [[getr]] */

/*s: function [[getnsc]] */
int
getnsc(void)
{
    int c;

    /*s: [[getnsc()]] peekc handling */
    if(peekc != IGN) {
        c = peekc;
        peekc = IGN;
    }
    /*e: [[getnsc()]] peekc handling */
    else
        c = GETC();

    for(;;) {
        if(c >= Runeself || !isspace(c))
            return c;
        if(c == '\n') {
            lineno++;
            return c;
        }
        // else, was a space, so continue
        c = GETC();
    }
    return -1; // unreachable
}
/*e: function [[getnsc]] */

/*s: function [[unget]] */
void
unget(int c)
{

    peekc = c;
    if(c == '\n')
        lineno--;
}
/*e: function [[unget]] */

/*s: function [[escchar]] */
long
escchar(long e, int longflg, bool escflg)
{
    long c, l;
    int i;

loop:
    c = getr();
    if(c == '\n') {
        yyerror("newline in string");
        return EOF;
    }
    if(c != '\\') {
        if(c == e)
            c = EOF;
        return c;
    }
    // else c is '\\'
    c = getr();

    /*s: [[escchar()]] if hexadecimal character */
    if(c == 'x') {
        /*
         * note this is not ansi,
         * supposed to only accept 2 hex
         */
        i = 2;
        if(longflg)
            i = 6;
        l = 0;
        for(; i>0; i--) {
            c = getc();
            if(c >= '0' && c <= '9') {
                l = l*16 + c-'0';
                continue;
            }
            if(c >= 'a' && c <= 'f') {
                l = l*16 + c-'a' + 10;
                continue;
            }
            if(c >= 'A' && c <= 'F') {
                l = l*16 + c-'A' + 10;
                continue;
            }
            unget(c);
            break;
        }
        if(escflg)
            l |= ESC;
        return l;
    }
    /*e: [[escchar()]] if hexadecimal character */
    /*s: [[escchar()]] if octal character */
    if(c >= '0' && c <= '7') {
        /*
         * note this is not ansi,
         * supposed to only accept 3 oct
         */
        i = 2;
        if(longflg)
            i = 8;
        l = c - '0';
        for(; i>0; i--) {
            c = getc();
            if(c >= '0' && c <= '7') {
                l = l*8 + c-'0';
                continue;
            }
            unget(c);
        }
        if(escflg)
            l |= ESC;
        return l;
    }
    /*e: [[escchar()]] if octal character */
    switch(c)
    {
    case '\n':	goto loop; // escaped newline

    case 'n':	return '\n';
    case 't':	return '\t';
    case 'b':	return '\b';
    case 'r':	return '\r';

    case 'f':	return '\f';
    case 'a':	return '\a';
    case 'v':	return '\v';
    }
    return c;
}
/*e: function [[escchar]] */

/*s: global [[itab]] */
struct
{
    char	*name;
    // enum<lexeme>
    ushort	lexical;
    // option<enum<Type_kind>>, None = 0 (or TXXX)
    ushort	type;
} itab[] =
{
    "auto",		LAUTO,		0,
    "static",	LSTATIC,	0,
    "extern",	LEXTERN,	0,
    "register",	LREGISTER,	0,

    "const",	LCONSTNT,	0,
    "volatile",	LVOLATILE,	0,

    "inline",	LINLINE,	0,
    "restrict",	LRESTRICT,	0,

    "void",		LVOID,		TVOID,
    "char",		LCHAR,		TCHAR,
    "int",		LINT,		TINT,
    "short",	LSHORT,		TSHORT,
    "long",		LLONG,		TLONG,
    "float",	LFLOAT,		TFLOAT,
    "double",	LDOUBLE,	TDOUBLE,

    "unsigned",	LUNSIGNED,	0,
    "signed",	LSIGNED,	0,

    "struct",	LSTRUCT,	0,
    "union",	LUNION,		0,
    "enum",		LENUM,		0,

    "typedef",	LTYPEDEF,	0,

    "if",		LIF,		0,
    "else",		LELSE,		0,
    "while",	LWHILE,		0,
    "do",		LDO,		0,
    "for",		LFOR,		0,
    "break",	LBREAK,		0,
    "continue",	LCONTINUE,	0,
    "switch",	LSWITCH,	0,
    "case",		LCASE,		0,
    "default",	LDEFAULT,	0,
    "return",	LRETURN,	0,
    "goto",		LGOTO,		0,

    "sizeof",	LSIZEOF,	0,
    /*s: [[itab]] entries, kencc extensions */
    "SET",		LSET,		0,
    "USED",		LUSED,		0,
    /*x: [[itab]] entries, kencc extensions */
    "typestr",	LTYPESTR,	0,
    /*x: [[itab]] entries, kencc extensions */
    "signof",	LSIGNOF,	0,
    /*e: [[itab]] entries, kencc extensions */
    0
};
/*e: global [[itab]] */

/*s: function [[cinit]] */
void
cinit(void)
{
    int i;
    /*s: [[cinit()]] locals */
    Sym *s;
    /*x: [[cinit()]] locals */
    Type *t;
    /*e: [[cinit()]] locals */

    /*s: [[cinit()]] lexing globals initialization */
    nerrors = 0;
    /*x: [[cinit()]] lexing globals initialization */
    lineno = 1;
    /*x: [[cinit()]] lexing globals initialization */
    iostack = I;
    iofree = I;
    /*x: [[cinit()]] lexing globals initialization */
    peekc = IGN;
    /*e: [[cinit()]] lexing globals initialization */
    /*s: [[cinit()]] memory globals initialization */
    nhunk = 0;
    /*e: [[cinit()]] memory globals initialization */
    /*s: [[cinit()]] types initialization */
    types[TXXX] = T;

    types[TCHAR]   = typ(TCHAR, T);
    types[TUCHAR]  = typ(TUCHAR, T);
    types[TSHORT]  = typ(TSHORT, T);
    types[TUSHORT] = typ(TUSHORT, T);
    types[TINT]    = typ(TINT, T);
    types[TUINT]   = typ(TUINT, T);
    types[TLONG]   = typ(TLONG, T);
    types[TULONG]  = typ(TULONG, T);
    types[TVLONG]  = typ(TVLONG, T);
    types[TUVLONG] = typ(TUVLONG, T);

    types[TFLOAT]  = typ(TFLOAT, T);
    types[TDOUBLE] = typ(TDOUBLE, T);

    types[TVOID]   = typ(TVOID, T);

    types[TENUM] = typ(TENUM, T);
    types[TFUNC] = typ(TFUNC, types[TINT]);
    types[TIND] = typ(TIND, types[TVOID]);
    /*e: [[cinit()]] types initialization */
    /*s: [[cinit()]] hash initialization */
    for(i=0; i<NHASH; i++)
        hash[i] = S;
    /*e: [[cinit()]] hash initialization */
    /*s: [[cinit()]] symbol table initialization */
    for(i=0; itab[i].name; i++) {
        s = slookup(itab[i].name);
        s->lexical = itab[i].lexical;
        if(itab[i].type != 0)
            s->type = types[itab[i].type];
    }
    /*e: [[cinit()]] symbol table initialization */
    /*s: [[cinit()]] namespace globals initialization */
    blockno = 0;
    autobn = blockno;
    /*x: [[cinit()]] namespace globals initialization */
    autoffset = 0;
    /*e: [[cinit()]] namespace globals initialization */
    /*s: [[cinit()]] dclstack initialization */
    dclstack = D;
    /*e: [[cinit()]] dclstack initialization */
    /*s: [[cinit()]] symstring initialization */
    symstring = slookup(".string");
    symstring->class = CSTATIC;
    t = typ(TARRAY, types[TCHAR]);
    t->width = 0;
    symstring->type = t;
    /*e: [[cinit()]] symstring initialization */
    /*s: [[cinit()]] nodproto initialization */
    nodproto = new(OPROTO, Z, Z);
    /*e: [[cinit()]] nodproto initialization */
    /*s: [[cinit()]] pathname initialisation from cwd */
    pathname = allocn(pathname, 0, 100);
    if(getwd(pathname, 99) == 0) {
        pathname = allocn(pathname, 100, 900);
        if(getwd(pathname, 999) == 0)
            strcpy(pathname, "/???");
    }
    /*e: [[cinit()]] pathname initialisation from cwd */
    /*s: [[cinit()]] fmtinstall */
    fmtinstall('O', Oconv);
    fmtinstall('T', Tconv);
    fmtinstall('F', FNconv);
    fmtinstall('Q', Qconv);
    fmtinstall('|', VBconv);
    /*x: [[cinit()]] fmtinstall */
    fmtinstall('L', Lconv);
    /*e: [[cinit()]] fmtinstall */
}
/*e: function [[cinit]] */

/*s: function [[filbuf]] */
int
filbuf(void)
{
    Io *i;

loop:
    i = iostack;
    if(i == I)
        return EOF;
    if(i->f < 0)
        goto pop;
    fi.c = read(i->f, i->b, BUFSIZ) - 1;
    if(fi.c < 0) {
        close(i->f);
        /*s: [[filbuf()]] when close file, call linehist */
        linehist(0, 0);
        /*e: [[filbuf()]] when close file, call linehist */
        goto pop;
    }
    fi.p = i->b + 1;
    return i->b[0] & 0xff;

/*s: [[filbuf()]] pop */
pop:
    // pop(iostack)
    iostack = i->link;
    // push(i, iofree)
    i->link = iofree;
    iofree = i;

    // i = top(iostack), the fresh top of the stack input file
    i = iostack;
    if(i == I)
        return EOF;
    // restore file pointers
    fi.p = i->p;
    fi.c = i->c;
    if(--fi.c < 0)
        goto loop;
    return *fi.p++ & 0xff;
/*e: [[filbuf()]] pop */
}
/*e: function [[filbuf]] */



/*s: function [[Oconv]] */
// enum<Node_kind> -> unit
int Oconv(Fmt *fp)
{
    int a;

    a = va_arg(fp->args, int);
    if(a < OXXX || a > OEND)
        return fmtprint(fp, "***badO %d***", a);

    return fmtstrcpy(fp, onames[a]);
}
/*e: function [[Oconv]] */

/*s: struct [[Atab]] */
struct Atab
    {
        Hist*	incl;	/* start of this include file */
        long	idel;	/* delta line number to apply to include */
        Hist*	line;	/* start of this #line directive */
        long	ldel;	/* delta line number to apply to #line */
};
/*e: struct [[Atab]] */

/*s: function [[Lconv]] */
// int -> string?
int
Lconv(Fmt *fp)
{
    char str[STRINGSZ], s[STRINGSZ];
    Hist *h;
    long l, d;
    int i, n;
    struct Atab  a[HISTSZ];

    l = va_arg(fp->args, long);
    n = 0;
    for(h = hist; h != H; h = h->link) {
        if(l < h->line)
            break;
        if(h->name) {
            if(h->offset != 0) {		/* #line directive, not #pragma */
                if(n > 0 && n < HISTSZ && h->offset >= 0) {
                    a[n-1].line = h;
                    a[n-1].ldel = h->line - h->offset + 1;
                }
            } else {
                if(n < HISTSZ) {	/* beginning of file */
                    a[n].incl = h;
                    a[n].idel = h->line;
                    a[n].line = 0;
                }
                n++;
            }
            continue;
        }
        n--;
        if(n > 0 && n < HISTSZ) {
            d = h->line - a[n].incl->line;
            a[n-1].ldel += d;
            a[n-1].idel += d;
        }
    }
    if(n > HISTSZ)
        n = HISTSZ;
    str[0] = 0;
    for(i=n-1; i>=0; i--) {
        if(i != n-1) {
            if(fp->flags & ~(FmtWidth|FmtPrec))	/* BUG ROB - was f3 */
                break;
            strcat(str, " ");
        }
        if(a[i].line)
            snprint(s, STRINGSZ, "%s:%ld[%s:%ld]",
                a[i].line->name, l-a[i].ldel+1,
                a[i].incl->name, l-a[i].idel+1);
        else
            snprint(s, STRINGSZ, "%s:%ld",
                a[i].incl->name, l-a[i].idel+1);
        if(strlen(s)+strlen(str) >= STRINGSZ-10)
            break;
        strcat(str, s);
        l = a[i].incl->line - 1;	/* now print out start of this file */
    }
    if(n == 0)
        strcat(str, "<eof>");

    return fmtstrcpy(fp, str);
}
/*e: function [[Lconv]] */

/*s: function [[Tconv]] */
// option<Type> -> string
int
Tconv(Fmt *fp)
{
    char str[STRINGSZ+20], s[STRINGSZ+20];
    Type *t, *t1;
    int et;
    long n;

    str[0] = '\0';
    for(t = va_arg(fp->args, Type*); t != T; t = t->link) {
        et = t->etype;
        if(str[0])
            strcat(str, " ");
        if(t->garb&~GINCOMPLETE) {
            sprint(s, "%s ", gnames[t->garb&~GINCOMPLETE]);
            if(strlen(str) + strlen(s) < STRINGSZ)
                strcat(str, s);
        }
        sprint(s, "%s", tnames[et]);
        if(strlen(str) + strlen(s) < STRINGSZ)
            strcat(str, s);
        if(et == TFUNC && (t1 = t->down)) {
            sprint(s, "(%T", t1);
            if(strlen(str) + strlen(s) < STRINGSZ)
                strcat(str, s);
            while(t1 = t1->down) {
                sprint(s, ", %T", t1);
                if(strlen(str) + strlen(s) < STRINGSZ)
                    strcat(str, s);
            }
            if(strlen(str) + strlen(s) < STRINGSZ)
                strcat(str, ")");
        }
        if(et == TARRAY) {
            n = t->width;
            if(t->link && t->link->width)
                n /= t->link->width;
            sprint(s, "[%ld]", n);
            if(strlen(str) + strlen(s) < STRINGSZ)
                strcat(str, s);
        }
        if(t->nbits) {
            sprint(s, " %d:%d", t->shift, t->nbits);
            if(strlen(str) + strlen(s) < STRINGSZ)
                strcat(str, s);
        }
        if(typesu[et]) {
            if(t->tag) {
                strcat(str, " ");
                if(strlen(str) + strlen(t->tag->name) < STRINGSZ)
                    strcat(str, t->tag->name);
            } else
                strcat(str, " {}");
            break;
        }
    }
    return fmtstrcpy(fp, str);
}
/*e: function [[Tconv]] */

/*s: function [[FNconv]] */
// option<Node identifier cases> -> string
int
FNconv(Fmt *fp)
{
    char *str;
    Node *n;

    n = va_arg(fp->args, Node*);
    str = "<indirect>";
    if(n != Z && (n->op == ONAME || n->op == ODOT || n->op == OELEM))
        str = n->sym->name;
    return fmtstrcpy(fp, str);
}
/*e: function [[FNconv]] */

/*s: function [[Qconv]] */
// ??
int
Qconv(Fmt *fp)
{
    char str[STRINGSZ+20], *s;
    long b;
    int i;

    str[0] = 0;
    for(b = va_arg(fp->args, long); b;) {
        i = bitno(b);
        if(str[0])
            strcat(str, " ");
        s = qnames[i];
        if(strlen(str) + strlen(s) >= STRINGSZ)
            break;
        strcat(str, s);
        b &= ~(1L << i);
    }
    return fmtstrcpy(fp, str);
}
/*e: function [[Qconv]] */

/*s: function [[VBconv]] */
// ??
int
VBconv(Fmt *fp)
{
    char str[STRINGSZ];
    int i, n, t, pc;

    n = va_arg(fp->args, int);
    pc = 0;	/* BUG: was printcol */
    i = 0;
    while(pc < n) {
        t = (pc+4) & ~3;
        if(t <= n) {
            str[i++] = '\t';
            pc = t;
            continue;
        }
        str[i++] = ' ';
        pc++;
    }
    str[i] = '\0';

    return fmtstrcpy(fp, str);
}
/*e: function [[VBconv]] */

/*s: function [[setinclude]] */
void
setinclude(char *p)
{
    int i;
    char *e, **np;

    while(*p != '\0') {
        e = strchr(p, ' ');
        if(e != nil)
            *e = '\0';

        // already there?
        for(i=0; i < ninclude; i++)
            if(strcmp(p, include[i]) == 0)
                break;
        // else
        if(i >= ninclude){
            /*s: [[setinclude()]] grow the array if necessary */
            // grow the array
            if(i >= maxinclude){
                maxinclude += 20;
                np = alloc(maxinclude * sizeof *np);
                if(include != nil)
                    memmove(np, include, (maxinclude - 20) * sizeof *np);
                include = np;
            }
            /*e: [[setinclude()]] grow the array if necessary */
            include[ninclude++] = p;
        }

        if(e == nil)
            break;
        p = e+1;
    }
}
/*e: function [[setinclude]] */
/*e: cc/lex.c */
