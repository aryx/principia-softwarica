/*s: assemblers/5a/lex.c */
#include "a.h"
#include "y.tab.h"

void	cinit(void);
int		assemble(char*);
void	cclean(void);
void	outhist(void);

/*s: function main(arm) */
void
main(int argc, char *argv[])
{
    char *p;
    int nout, nproc, status, i, c;

    thechar = '5';
    thestring = "arm";
    memset(debug, 0, sizeof(debug));

    cinit();
    outfile = nil;
    include[ninclude++] = ".";

    ARGBEGIN {
    default:
        c = ARGC();
        if(c >= 0 || c < sizeof(debug))
            debug[c] = 1;
        break;

    case 'o':
        outfile = ARGF();
        break;

    case 'D':
        p = ARGF();
        if(p)
            Dlist[nDlist++] = p;
        break;

    case 'I':
        p = ARGF();
        setinclude(p);
        break;
    case 't':
        thechar = 't';
        thestring = "thumb";
        break;
    } ARGEND

    if(*argv == 0) {
        print("usage: %ca [-options] file.s\n", thechar);
        errorexit();
    }
    if(argc > 1) {
        nproc = 1;
        if(p = getenv("NPROC"))
            nproc = atol(p);	/* */
        c = 0;
        nout = 0;
        for(;;) {
            while(nout < nproc && argc > 0) {
                i = fork();
                if(i < 0) {
                    i = mywait(&status);
                    if(i < 0)
                        errorexit();
                    if(status)
                        c++;
                    nout--;
                    continue;
                }
                if(i == 0) {
                    print("%s:\n", *argv);
                    if(assemble(*argv))
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
    if(assemble(argv[0]))
        errorexit();
    exits(0);
}
/*e: function main(arm) */

/*s: function assemble(arm) */
int
assemble(char *file)
{
    char ofile[100], incfile[20], *p;
    int i, of;

    strcpy(ofile, file);
    p = utfrrune(ofile, pathchar());
    if(p) {
        include[0] = ofile;
        *p++ = 0;
    } else
        p = ofile;
    if(outfile == 0) {
        outfile = p;
        if(outfile){
            p = utfrrune(outfile, '.');
            if(p)
                if(p[1] == 's' && p[2] == 0)
                    p[0] = 0;
            p = utfrune(outfile, 0);
            p[0] = '.';
            p[1] = thechar;
            p[2] = 0;
        } else
            outfile = "/dev/null";
    }
    p = getenv("INCLUDE");
    if(p) {
        setinclude(p);
    } else {
        if(systemtype(Plan9)) {
            sprint(incfile,"/%s/include", thestring);
            setinclude(strdup(incfile));
        }
    }

    of = mycreat(outfile, 0664);
    if(of < 0) {
        yyerror("%ca: cannot create %s", thechar, outfile);
        errorexit();
    }
    Binit(&obuf, of, OWRITE);

    pass = 1;
    pinit(file);
    for(i=0; i<nDlist; i++)
        dodefine(Dlist[i]);
    yyparse();
    if(nerrors) {
        cclean();
        return nerrors;
    }

    pass = 2;
    outhist();
    pinit(file);
    for(i=0; i<nDlist; i++)
        dodefine(Dlist[i]);
    yyparse();
    cclean();
    return nerrors;
}
/*e: function assemble(arm) */

/*s: struct Itab(arm) */
struct Itab
{
    char	*name;

    //enum<token_kind> ???
    ushort	type;
    //enum<opcode_kind> | enum<operand_kind> ??
    ushort	value;
};
/*e: struct Itab(arm) */

/*s: global itab(arm) */
struct Itab itab[] =
{
    "SP",		LSP,	D_AUTO,
    "SB",		LSB,	D_EXTERN,
    "FP",		LFP,	D_PARAM,
    "PC",		LPC,	D_BRANCH,

    "R",		LR,	0,
    "R0",		LREG,	0,
    "R1",		LREG,	1,
    "R2",		LREG,	2,
    "R3",		LREG,	3,
    "R4",		LREG,	4,
    "R5",		LREG,	5,
    "R6",		LREG,	6,
    "R7",		LREG,	7,
    "R8",		LREG,	8,
    "R9",		LREG,	9,
    "R10",		LREG,	10,
    "R11",		LREG,	11,
    "R12",		LREG,	12,
    "R13",		LREG,	13,
    "R14",		LREG,	14,
    "R15",		LREG,	15,

    "F",		LF,	0,

    "F0",		LFREG,	0,
    "F1",		LFREG,	1,
    "F2",		LFREG,	2,
    "F3",		LFREG,	3,
    "F4",		LFREG,	4,
    "F5",		LFREG,	5,
    "F6",		LFREG,	6,
    "F7",		LFREG,	7,
    "F8",		LFREG,	8,
    "F9",		LFREG,	9,
    "F10",		LFREG,	10,
    "F11",		LFREG,	11,
    "F12",		LFREG,	12,
    "F13",		LFREG,	13,
    "F14",		LFREG,	14,
    "F15",		LFREG,	15,

    "C",		LC,	0,

    "C0",		LCREG,	0,
    "C1",		LCREG,	1,
    "C2",		LCREG,	2,
    "C3",		LCREG,	3,
    "C4",		LCREG,	4,
    "C5",		LCREG,	5,
    "C6",		LCREG,	6,
    "C7",		LCREG,	7,
    "C8",		LCREG,	8,
    "C9",		LCREG,	9,
    "C10",		LCREG,	10,
    "C11",		LCREG,	11,
    "C12",		LCREG,	12,
    "C13",		LCREG,	13,
    "C14",		LCREG,	14,
    "C15",		LCREG,	15,

    "CPSR",		LPSR,	0,
    "SPSR",		LPSR,	1,

    "FPSR",		LFCR,	0,
    "FPCR",		LFCR,	1,

    ".EQ",		LCOND,	0,
    ".NE",		LCOND,	1,
    ".CS",		LCOND,	2,
    ".HS",		LCOND,	2,
    ".CC",		LCOND,	3,
    ".LO",		LCOND,	3,
    ".MI",		LCOND,	4,
    ".PL",		LCOND,	5,
    ".VS",		LCOND,	6,
    ".VC",		LCOND,	7,
    ".HI",		LCOND,	8,
    ".LS",		LCOND,	9,
    ".GE",		LCOND,	10,
    ".LT",		LCOND,	11,
    ".GT",		LCOND,	12,
    ".LE",		LCOND,	13,
    ".AL",		LCOND,	Always,

    ".U",		LS,	C_UBIT,
    ".S",		LS,	C_SBIT,
    ".W",		LS,	C_WBIT,
    ".P",		LS,	C_PBIT,
    ".PW",		LS,	C_WBIT|C_PBIT,
    ".WP",		LS,	C_WBIT|C_PBIT,

    ".F",		LS,	C_FBIT,

    ".IBW",		LS,	C_WBIT|C_PBIT|C_UBIT,
    ".IAW",		LS,	C_WBIT|C_UBIT,
    ".DBW",		LS,	C_WBIT|C_PBIT,
    ".DAW",		LS,	C_WBIT,
    ".IB",		LS,	C_PBIT|C_UBIT,
    ".IA",		LS,	C_UBIT,
    ".DB",		LS,	C_PBIT,
    ".DA",		LS,	0,

    "@",		LAT,	0,

    "AND",		LTYPE1,	AAND,
    "EOR",		LTYPE1,	AEOR,
    "SUB",		LTYPE1,	ASUB,
    "RSB",		LTYPE1,	ARSB,
    "ADD",		LTYPE1,	AADD,
    "ADC",		LTYPE1,	AADC,
    "SBC",		LTYPE1,	ASBC,
    "RSC",		LTYPE1,	ARSC,
    "ORR",		LTYPE1,	AORR,
    "BIC",		LTYPE1,	ABIC,

    "SLL",		LTYPE1,	ASLL,
    "SRL",		LTYPE1,	ASRL,
    "SRA",		LTYPE1,	ASRA,

    "MUL",		LTYPE1, AMUL,
    "MULA",		LTYPEN, AMULA,
    "DIV",		LTYPE1,	ADIV,
    "MOD",		LTYPE1,	AMOD,

    "MULL",		LTYPEM, AMULL,
    "MULAL",	LTYPEM, AMULAL,
    "MULLU",	LTYPEM, AMULLU,
    "MULALU",	LTYPEM, AMULALU,

    "MVN",		LTYPE2, AMVN,	/* op2 ignored */

    "MOVB",		LTYPE3, AMOVB,
    "MOVBU",	LTYPE3, AMOVBU,
    "MOVH",		LTYPE3, AMOVH,
    "MOVHU",	LTYPE3, AMOVHU,
    "MOVW",		LTYPE3, AMOVW,

    "MOVD",		LTYPE3, AMOVD,
    "MOVDF",		LTYPE3, AMOVDF,
    "MOVDW",	LTYPE3, AMOVDW,
    "MOVF",		LTYPE3, AMOVF,
    "MOVFD",		LTYPE3, AMOVFD,
    "MOVFW",		LTYPE3, AMOVFW,
    "MOVWD",	LTYPE3, AMOVWD,
    "MOVWF",		LTYPE3, AMOVWF,

    "LDREX",		LTYPE3, ALDREX,
    "LDREXD",		LTYPE3, ALDREXD,
    "STREX",		LTYPE9, ASTREX,
    "STREXD",		LTYPE9, ASTREXD,


    "SQRTF",	LTYPEI, ASQRTF,
    "SQRTD",	LTYPEI, ASQRTD,
    "CMPF",		LTYPEL, ACMPF,
    "CMPD",		LTYPEL, ACMPD,
    "ADDF",		LTYPEK,	AADDF,
    "ADDD",		LTYPEK,	AADDD,
    "SUBF",		LTYPEK,	ASUBF,
    "SUBD",		LTYPEK,	ASUBD,
    "MULF",		LTYPEK,	AMULF,
    "MULD",		LTYPEK,	AMULD,
    "DIVF",		LTYPEK,	ADIVF,
    "DIVD",		LTYPEK,	ADIVD,

    "B",		LTYPE4, AB,
    "BL",		LTYPE4, ABL,
    "BX",		LTYPEBX,	ABX,

    "BEQ",		LTYPE5,	ABEQ,
    "BNE",		LTYPE5,	ABNE,
    "BCS",		LTYPE5,	ABCS,
    "BHS",		LTYPE5,	ABHS,
    "BCC",		LTYPE5,	ABCC,
    "BLO",		LTYPE5,	ABLO,
    "BMI",		LTYPE5,	ABMI,
    "BPL",		LTYPE5,	ABPL,
    "BVS",		LTYPE5,	ABVS,
    "BVC",		LTYPE5,	ABVC,
    "BHI",		LTYPE5,	ABHI,
    "BLS",		LTYPE5,	ABLS,
    "BGE",		LTYPE5,	ABGE,
    "BLT",		LTYPE5,	ABLT,
    "BGT",		LTYPE5,	ABGT,
    "BLE",		LTYPE5,	ABLE,
    "BCASE",	LTYPE5,	ABCASE,

    "SWI",		LTYPE6, ASWI,

    "CMP",		LTYPE7,	ACMP,
    "TST",		LTYPE7,	ATST,
    "TEQ",		LTYPE7,	ATEQ,
    "CMN",		LTYPE7,	ACMN,

    "MOVM",		LTYPE8, AMOVM,

    "SWPBU",	LTYPE9, ASWPBU,
    "SWPW",		LTYPE9, ASWPW,

    "RET",		LTYPEA, ARET,
    "RFE",		LTYPEA, ARFE,

    "TEXT",		LTYPEB, ATEXT,
    "GLOBL",	LTYPEB, AGLOBL,
    "DATA",		LTYPEC, ADATA,
    "CASE",		LTYPED, ACASE,
    "END",		LTYPEE, AEND,
    "WORD",		LTYPEH, AWORD,
    "NOP",		LTYPEI, ANOP,

    "MCR",		LTYPEJ, 0,
    "MRC",		LTYPEJ, 1,
    0
};
/*e: global itab(arm) */

/*s: function cinit(arm) */
void
cinit(void)
{
    Sym *s;
    int i;

    nullgen.sym = S;
    nullgen.offset = 0;

    nullgen.type = D_NONE;
    nullgen.name = D_NONE;
    nullgen.reg = NREG;

    if(FPCHIP)
        nullgen.dval = 0;
    for(i=0; i<sizeof(nullgen.sval); i++)
        nullgen.sval[i] = 0;

    nerrors = 0;
    iostack = I;
    iofree = I;
    peekc = IGN;
    nhunk = 0;
    for(i=0; i<NHASH; i++)
        hash[i] = S;
    for(i=0; itab[i].name; i++) {
        s = slookup(itab[i].name);
        s->type = itab[i].type;
        s->value = itab[i].value;
    }

    pathname = allocn(pathname, 0, 100);
    if(getwd(pathname, 99) == 0) {
        pathname = allocn(pathname, 100, 900);
        if(getwd(pathname, 999) == 0)
            strcpy(pathname, "/???");
    }
}
/*e: function cinit(arm) */

/*s: function syminit */
void
syminit(Sym *s)
{

    s->type = LNAME;
    s->value = 0;
}
/*e: function syminit */

/*s: function cclean(arm) */
void
cclean(void)
{

    outcode(AEND, Always, &nullgen, NREG, &nullgen);
    Bflush(&obuf);
}
/*e: function cclean(arm) */

/*s: function zname(arm) */
void
zname(char *n, int t, int s)
{

    Bputc(&obuf, ANAME);
    Bputc(&obuf, t);	/* type */
    Bputc(&obuf, s);	/* sym */
    while(*n) {
        Bputc(&obuf, *n);
        n++;
    }
    Bputc(&obuf, 0);
}
/*e: function zname(arm) */

/*s: function zaddr(arm) */
void
zaddr(Gen *a, int s)
{
    long l;
    int i;
    char *n;
    Ieee e;

    Bputc(&obuf, a->type);
    Bputc(&obuf, a->reg);
    Bputc(&obuf, s);
    Bputc(&obuf, a->name);
    switch(a->type) {
    default:
        print("unknown type %d\n", a->type);
        exits("arg");

    case D_NONE:
    case D_REG:
    case D_FREG:
    case D_PSR:
    case D_FPCR:
        break;

    case D_REGREG:
        Bputc(&obuf, a->offset);
        break;

    case D_OREG:
    case D_CONST:
    case D_BRANCH:
    case D_SHIFT:
        l = a->offset;
        Bputc(&obuf, l);
        Bputc(&obuf, l>>8);
        Bputc(&obuf, l>>16);
        Bputc(&obuf, l>>24);
        break;

    case D_SCONST:
        n = a->sval;
        for(i=0; i<NSNAME; i++) {
            Bputc(&obuf, *n);
            n++;
        }
        break;

    case D_FCONST:
        ieeedtod(&e, a->dval);
        Bputc(&obuf, e.l);
        Bputc(&obuf, e.l>>8);
        Bputc(&obuf, e.l>>16);
        Bputc(&obuf, e.l>>24);
        Bputc(&obuf, e.h);
        Bputc(&obuf, e.h>>8);
        Bputc(&obuf, e.h>>16);
        Bputc(&obuf, e.h>>24);
        break;
    }
}
/*e: function zaddr(arm) */

/*s: global bcode(arm) */
static int bcode[] =
{
    ABEQ,
    ABNE,
    ABCS,
    ABCC,
    ABMI,
    ABPL,
    ABVS,
    ABVC,
    ABHI,
    ABLS,
    ABGE,
    ABLT,
    ABGT,
    ABLE,
    AB,
    ANOP,
};
/*e: global bcode(arm) */

/*s: function outcode(arm) */
void
outcode(int a, int scond, Gen *g1, int reg, Gen *g2)
{
    int sf, st, t;
    Sym *s;

    /* hack to make B.NE etc. work: turn it into the corresponding conditional */
    if(a == AB){
        a = bcode[scond&0xf];
        scond = (scond & ~0xf) | Always;
    }

    if(pass == 1)
        goto out;
jackpot:
    sf = 0;
    s = g1->sym;
    while(s != S) {
        sf = s->symidx;
        if(sf < 0 || sf >= NSYM)
            sf = 0;
        t = g1->name;
        if(h[sf].type == t)
        if(h[sf].sym == s)
            break;
        zname(s->name, t, symcounter);
        s->symidx = symcounter;
        h[symcounter].sym = s;
        h[symcounter].type = t;
        sf = symcounter;
        symcounter++;
        if(symcounter >= NSYM)
            symcounter = 1;
        break;
    }
    st = 0;
    s = g2->sym;
    while(s != S) {
        st = s->symidx;
        if(st < 0 || st >= NSYM)
            st = 0;
        t = g2->name;
        if(h[st].type == t)
        if(h[st].sym == s)
            break;
        zname(s->name, t, symcounter);
        s->symidx = symcounter;
        h[symcounter].sym = s;
        h[symcounter].type = t;
        st = symcounter;
        symcounter++;
        if(symcounter >= NSYM)
            symcounter = 1;
        if(st == sf)
            goto jackpot;
        break;
    }
    Bputc(&obuf, a);
    Bputc(&obuf, scond);
    Bputc(&obuf, reg);
    Bputc(&obuf, lineno);
    Bputc(&obuf, lineno>>8);
    Bputc(&obuf, lineno>>16);
    Bputc(&obuf, lineno>>24);
    zaddr(g1, sf);
    zaddr(g2, st);

out:
    if(a != AGLOBL && a != ADATA)
        pc++;
}
/*e: function outcode(arm) */

/*s: function outhist(arm) */
void
outhist(void)
{
    Gen g;
    Hist *h;
    char *p, *q, *op, c;
    int n;

    g = nullgen;
    c = pathchar();
    for(h = hist; h != H; h = h->link) {
        p = h->name;
        op = 0;
        if(p && p[0] != c && h->offset == 0 && pathname){
            if(pathname[0] == c){
                op = p;
                p = pathname;
            }
        }
        while(p) {
            q = strchr(p, c);
            if(q) {
                n = q-p;
                if(n == 0){
                    n = 1;	/* leading "/" */
                    *p = '/';	/* don't emit "\" on windows */
                }
                q++;
            } else {
                n = strlen(p);
                q = 0;
            }
            if(n) {
                Bputc(&obuf, ANAME);
                Bputc(&obuf, D_FILE);	/* type */
                Bputc(&obuf, 1);	/* sym */
                Bputc(&obuf, '<');
                Bwrite(&obuf, p, n);
                Bputc(&obuf, 0);
            }
            p = q;
            if(p == 0 && op) {
                p = op;
                op = 0;
            }
        }
        g.offset = h->offset;

        Bputc(&obuf, AHISTORY);
        Bputc(&obuf, Always);
        Bputc(&obuf, 0);
        Bputc(&obuf, h->line);
        Bputc(&obuf, h->line>>8);
        Bputc(&obuf, h->line>>16);
        Bputc(&obuf, h->line>>24);
        zaddr(&nullgen, 0);
        zaddr(&g, 0);
    }
}
/*e: function outhist(arm) */

// now use aa.a8
//#include "../cc/lexbody"
//#include "../cc/compat"

// used to be in ../cc/lexbody and factorized between assemblers by
// using #include, but ugly, so I copy pasted the function for now
/*s: function yylex(arm) */
long
yylex(void)
{
    int c, c1;
    char *cp;
    Sym *s;

    c = peekc;
    if(c != IGN) {
        peekc = IGN;
        goto l1;
    }
l0:
    c = GETC();

l1:
    if(c == EOF) {
        peekc = EOF;
        return -1;
    }
    if(isspace(c)) {
        if(c == '\n') {
            lineno++;
            return ';';
        }
        goto l0;
    }
    if(isalpha(c))
        goto talph;
    if(isdigit(c))
        goto tnum;
    switch(c)
    {
    case '\n':
        lineno++;
        return ';';

    case '#':
        domacro();
        goto l0;

    case '.':
        c = GETC();
        if(isalpha(c)) {
            cp = symb;
            *cp++ = '.';
            goto aloop;
        }
        if(isdigit(c)) {
            cp = symb;
            *cp++ = '.';
            goto casedot;
        }
        peekc = c;
        return '.';

    talph:
    case '_':
    case '@':
        cp = symb;

    aloop:
        *cp++ = c;
        c = GETC();
        if(isalpha(c) || isdigit(c) || c == '_' || c == '$')
            goto aloop;
        *cp = 0;
        peekc = c;
        s = lookup();
        if(s->macro) {
            newio();
            cp = ionext->b;
            macexpand(s, cp);
            pushio();
            ionext->link = iostack;
            iostack = ionext;
            fi.p = cp;
            fi.c = strlen(cp);
            if(peekc != IGN) {
                cp[fi.c++] = peekc;
                cp[fi.c] = 0;
                peekc = IGN;
            }
            goto l0;
        }
        if(s->type == 0)
            s->type = LNAME;
        if(s->type == LNAME ||
           s->type == LVAR ||
           s->type == LLAB) {
            yylval.sym = s;
            return s->type;
        }
        yylval.lval = s->value;
        return s->type;

    tnum:
        cp = symb;
        if(c != '0')
            goto dc;
        *cp++ = c;
        c = GETC();
        c1 = 3;
        if(c == 'x' || c == 'X') {
            c1 = 4;
            c = GETC();
        } else
        if(c < '0' || c > '7')
            goto dc;
        yylval.lval = 0;
        for(;;) {
            if(c >= '0' && c <= '9') {
                if(c > '7' && c1 == 3)
                    break;
                yylval.lval <<= c1;
                yylval.lval += c - '0';
                c = GETC();
                continue;
            }
            if(c1 == 3)
                break;
            if(c >= 'A' && c <= 'F')
                c += 'a' - 'A';
            if(c >= 'a' && c <= 'f') {
                yylval.lval <<= c1;
                yylval.lval += c - 'a' + 10;
                c = GETC();
                continue;
            }
            break;
        }
        goto ncu;

    dc:
        for(;;) {
            if(!isdigit(c))
                break;
            *cp++ = c;
            c = GETC();
        }
        if(c == '.')
            goto casedot;
        if(c == 'e' || c == 'E')
            goto casee;
        *cp = 0;
        if(sizeof(yylval.lval) == sizeof(vlong))
            yylval.lval = strtoll(symb, nil, 10);
        else
            yylval.lval = strtol(symb, nil, 10);

    ncu:
        while(c == 'U' || c == 'u' || c == 'l' || c == 'L')
            c = GETC();
        peekc = c;
        return LCONST;

    casedot:
        for(;;) {
            *cp++ = c;
            c = GETC();
            if(!isdigit(c))
                break;
        }
        if(c == 'e' || c == 'E')
            goto casee;
        goto caseout;

    casee:
        *cp++ = 'e';
        c = GETC();
        if(c == '+' || c == '-') {
            *cp++ = c;
            c = GETC();
        }
        while(isdigit(c)) {
            *cp++ = c;
            c = GETC();
        }

    caseout:
        *cp = 0;
        peekc = c;
        if(FPCHIP) {
            yylval.dval = atof(symb);
            return LFCONST;
        }
        yyerror("assembler cannot interpret fp constants");
        yylval.lval = 1L;
        return LCONST;

    case '"':
        memcpy(yylval.sval, nullgen.sval, sizeof(yylval.sval));
        cp = yylval.sval;
        c1 = 0;
        for(;;) {
            c = escchar('"');
            if(c == EOF)
                break;
            if(c1 < sizeof(yylval.sval))
                *cp++ = c;
            c1++;
        }
        if(c1 > sizeof(yylval.sval))
            yyerror("string constant too long");
        return LSCONST;

    case '\'':
        c = escchar('\'');
        if(c == EOF)
            c = '\'';
        if(escchar('\'') != EOF)
            yyerror("missing '");
        yylval.lval = c;
        return LCONST;

    case '/':
        c1 = GETC();
        if(c1 == '/') {
            for(;;) {
                c = GETC();
                if(c == '\n')
                    goto l1;
                if(c == EOF) {
                    yyerror("eof in comment");
                    errorexit();
                }
            }
        }
        if(c1 == '*') {
            for(;;) {
                c = GETC();
                while(c == '*') {
                    c = GETC();
                    if(c == '/')
                        goto l0;
                }
                if(c == EOF) {
                    yyerror("eof in comment");
                    errorexit();
                }
                if(c == '\n')
                    lineno++;
            }
        }
        break;

    default:
        return c;
    }
    peekc = c1;
    return c;
}
/*e: function yylex(arm) */

// #include "../cc/macbody"
/*e: assemblers/5a/lex.c */
