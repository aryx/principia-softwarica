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
    /*s: [[main()]] locals */
    char *p;
    /*x: [[main()]] locals */
    int nout, nproc, status;
    int i, c;
    /*e: [[main()]] locals */
    /*s: [[main()]] debug initialization */
    memset(debug, false, sizeof(debug));
    /*e: [[main()]] debug initialization */

    thechar = '5';
    thestring = "arm";

    cinit();
    include[ninclude++] = ".";

    ARGBEGIN {
    /*s: [[main()]] command line processing */
    case 'o':
        outfile = ARGF();
        break;
    /*x: [[main()]] command line processing */
    case 'I':
        p = ARGF();
        setinclude(p);
        break;
    /*x: [[main()]] command line processing */
    case 'D':
        p = ARGF();
        if(p)
            Dlist[nDlist++] = p;
        break;
    /*x: [[main()]] command line processing */
    default:
        c = ARGC();
        if(c >= 0 || c < sizeof(debug))
            debug[c] = true;
        break;
    /*e: [[main()]] command line processing */
    } ARGEND

    if(*argv == 0) {
        print("usage: %ca [-options] file.s\n", thechar);
        errorexit();
    }

    /*s: [[main()]] multiple files handling */
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
    /*e: [[main()]] multiple files handling */

    if(assemble(argv[0]))
        errorexit();
    exits(0);
}
/*e: function main(arm) */

/*s: function assemble */
int
assemble(char *infile)
{
    /*s: [[assemble()]] locals */
    char *p;
    fdt of; // outfile
    int i;
    /*x: [[assemble()]] locals */
    char ofile[100];
    /*x: [[assemble()]] locals */
    char incfile[20];
    /*e: [[assemble()]] locals */

    /*s: [[assemble()]] set p to basename(infile) and adjust include */
    // p = basename(infile)
    // include[0] = dirname(infile); 
    strcpy(ofile, infile);
    p = utfrrune(ofile, '/');
    if(p) {
        include[0] = ofile;
        *p++ = '\0';
    } else
        p = ofile;
    /*e: [[assemble()]] set p to basename(infile) and adjust include */
    if(outfile == nil) {
        /*s: [[assemble()]] set outfile to {basename(infile)}.{thechar} */
        // outfile =  p =~ s/.s/.5/;
        outfile = p;
        if(outfile){
            p = utfrrune(outfile, '.');
            if(p)
                if(p[1] == 's' && p[2] == '\0')
                    p[0] = '\0';
            p = utfrune(outfile, '\0');
            p[0] = '.';
            p[1] = thechar;
            p[2] = '\0';
        } else
            outfile = "/dev/null";
        /*e: [[assemble()]] set outfile to {basename(infile)}.{thechar} */
    }
    /*s: [[assemble()]] setinclude("/{thestring}/include") or INCLUDE */
    p = getenv("INCLUDE");
    if(p) {
        setinclude(p);
    } else {
        if(systemtype(Plan9)) {
            sprint(incfile,"/%s/include", thestring);
            setinclude(strdup(incfile));
        }
    }
    /*e: [[assemble()]] setinclude("/{thestring}/include") or INCLUDE */

    of = mycreat(outfile, 0664);
    if(of < 0) {
        yyerror("%ca: cannot create %s", thechar, outfile);
        errorexit();
    }
    Binit(&obuf, of, OWRITE);

    pass = 1;

    pinit(infile);
    /*s: [[assemble()]] init Dlist after pinit */
    for(i=0; i<nDlist; i++)
            dodefine(Dlist[i]);
    /*e: [[assemble()]] init Dlist after pinit */
    yyparse(); // calls outcode() but does nothing when pass == 1

    if(nerrors) {
        cclean();
        return nerrors;
    }

    pass = 2;
    outhist(); // header

    pinit(infile);
    /*s: [[assemble()]] init Dlist after pinit */
    for(i=0; i<nDlist; i++)
            dodefine(Dlist[i]);
    /*e: [[assemble()]] init Dlist after pinit */
    yyparse(); // calls outcode() that now does things

    cclean();
    return nerrors;
}
/*e: function assemble */

/*s: struct Itab(arm) */
struct Itab
{
    char	*name;

    //enum<token_kind>
    ushort	type;
    //enum<opcode|operand_kind|sym_kind|registr> | int
    ushort	value;
};
/*e: struct Itab(arm) */

/*s: global itab(arm) */
// map<string, (token_kind * enum<opcode|register|...>)>
struct Itab itab[] =
{
    "NOP",		LMISC, ANOP,
    /*s: [[itab]] elements */
    "AND",		LARITH,	AAND,
    "ORR",		LARITH,	AORR,
    "EOR",		LARITH,	AEOR,
    /*x: [[itab]] elements */
    "ADD",		LARITH,	AADD,
    "SUB",		LARITH,	ASUB,
    /*x: [[itab]] elements */
    "BIC",		LARITH,	ABIC,
    "RSB",		LARITH,	ARSB,
    "ADC",		LARITH,	AADC,
    "SBC",		LARITH,	ASBC,
    "RSC",		LARITH,	ARSC,
    /*x: [[itab]] elements */
    "SLL",		LARITH,	ASLL,
    "SRL",		LARITH,	ASRL,
    "SRA",		LARITH,	ASRA,
    /*x: [[itab]] elements */
    "MUL",		LARITH, AMUL,
    "DIV",		LARITH,	ADIV,
    "MOD",		LARITH,	AMOD,
    /*x: [[itab]] elements */
    "CMP",		LCMP,	ACMP,
    "TST",		LCMP,	ATST,
    "TEQ",		LCMP,	ATEQ,
    /*x: [[itab]] elements */
    "CMN",		LCMP,	ACMN,
    /*x: [[itab]] elements */
    "MOVW",		LMOV, AMOVW,
    "MOVB",		LMOV, AMOVB,
    "MOVBU",	LMOV, AMOVBU,
    "MOVH",		LMOV, AMOVH,
    "MOVHU",	LMOV, AMOVHU,
    /*x: [[itab]] elements */
    "MVN",		LMVN, AMVN,	/* op2 ignored */
    /*x: [[itab]] elements */
    "SWPW",		LSWAP, ASWPW,
    "SWPBU",	LSWAP, ASWPBU,
    /*x: [[itab]] elements */
    "B",		LBRANCH, AB,
    "BL",		LBRANCH, ABL,
    /*x: [[itab]] elements */
    "BEQ",		LBCOND,	ABEQ,
    "BNE",		LBCOND,	ABNE,
    "BHS",		LBCOND,	ABHS,
    "BLO",		LBCOND,	ABLO,
    "BMI",		LBCOND,	ABMI,
    "BPL",		LBCOND,	ABPL,
    "BVS",		LBCOND,	ABVS,
    "BVC",		LBCOND,	ABVC,
    "BHI",		LBCOND,	ABHI,
    "BLS",		LBCOND,	ABLS,
    "BGE",		LBCOND,	ABGE,
    "BLT",		LBCOND,	ABLT,
    "BGT",		LBCOND,	ABGT,
    "BLE",		LBCOND,	ABLE,
    /*x: [[itab]] elements */
    "RET",		LRET, ARET,
    /*x: [[itab]] elements */
    "SWI",		LSWI, ASWI,
    /*x: [[itab]] elements */
    "RFE",		LRET, ARFE,
    /*x: [[itab]] elements */
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
    /*x: [[itab]] elements */
    ".U",		LS,	C_UBIT,
    ".S",		LS,	C_SBIT,
    ".W",		LS,	C_WBIT,
    ".P",		LS,	C_PBIT,
    /*x: [[itab]] elements */
    ".PW",		LS,	C_WBIT|C_PBIT,
    ".WP",		LS,	C_WBIT|C_PBIT,
    /*x: [[itab]] elements */
    ".IBW",		LS,	C_WBIT|C_PBIT|C_UBIT,
    ".IAW",		LS,	C_WBIT|C_UBIT,
    ".DBW",		LS,	C_WBIT|C_PBIT,
    ".DAW",		LS,	C_WBIT,

    ".IB",		LS,	C_PBIT|C_UBIT,
    ".IA",		LS,	C_UBIT,
    ".DB",		LS,	C_PBIT,
    ".DA",		LS,	0,
    /*x: [[itab]] elements */
    ".F",		LS,	C_FBIT,
    /*x: [[itab]] elements */
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
    /*x: [[itab]] elements */
    "R",		LR,	0,
    /*x: [[itab]] elements */
    "@",		LAT,	0,
    /*x: [[itab]] elements */
    "SB",		LSB,	D_EXTERN,
    "SP",		LSP,	D_AUTO,
    "FP",		LFP,	D_PARAM,
    /*x: [[itab]] elements */
    "PC",		LPC,	D_BRANCH,
    /*x: [[itab]] elements */
    "CPSR",		LPSR,	0,
    "SPSR",		LPSR,	1,
    /*x: [[itab]] elements */
    "TEXT",		LDEF, ATEXT,
    "GLOBL",	LDEF, AGLOBL,
    /*x: [[itab]] elements */
    "DATA",		LDATA, ADATA,
    "WORD",		LWORD, AWORD,
    /*x: [[itab]] elements */
    "END",		LEND, AEND,
    /*x: [[itab]] elements */
    "MULL",		LMULL, AMULL,
    "MULAL",	LMULL, AMULAL,
    "MULLU",	LMULL, AMULLU,
    "MULALU",	LMULL, AMULALU,
    /*x: [[itab]] elements */
    "MULA",		LMULA, AMULA,
    /*x: [[itab]] elements */
    "MOVD",		LMOV, AMOVD,
    "MOVDF",	LMOV, AMOVDF,
    "MOVDW",	LMOV, AMOVDW,
    "MOVF",		LMOV, AMOVF,
    "MOVFD",	LMOV, AMOVFD,
    "MOVFW",	LMOV, AMOVFW,
    "MOVWD",	LMOV, AMOVWD,
    "MOVWF",	LMOV, AMOVWF,
    /*x: [[itab]] elements */
    "SQRTF",	LMISC, ASQRTF,
    "SQRTD",	LMISC, ASQRTD,
    "CMPF",		LCMPFLOAT, ACMPF,
    "CMPD",		LCMPFLOAT, ACMPD,
    "ADDF",		LARITHFLOAT,	AADDF,
    "ADDD",		LARITHFLOAT,	AADDD,
    "SUBF",		LARITHFLOAT,	ASUBF,
    "SUBD",		LARITHFLOAT,	ASUBD,
    "MULF",		LARITHFLOAT,	AMULF,
    "MULD",		LARITHFLOAT,	AMULD,
    "DIVF",		LARITHFLOAT,	ADIVF,
    "DIVD",		LARITHFLOAT,	ADIVD,
    /*x: [[itab]] elements */
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
    /*x: [[itab]] elements */
    "FPSR",		LFCR,	0,
    "FPCR",		LFCR,	1,
    /*x: [[itab]] elements */
    "MOVM",		LMOVM, AMOVM,
    /*x: [[itab]] elements */
    "MCR",		LSYSTEM, 0,
    "MRC",		LSYSTEM, 1,
    /*x: [[itab]] elements */
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
    /*e: [[itab]] elements */
    0
};
/*e: global itab(arm) */

/*s: function cinit(arm) */
void
cinit(void)
{
    Sym *s;
    int i;

    /*s: [[cinit()]] nullgen initialisation */
    nullgen.type = D_NONE;
    nullgen.reg = R_NONE;
    nullgen.sym = S;
    nullgen.symkind = N_NONE;
    nullgen.offset = 0;
    if(FPCHIP)
        nullgen.dval = 0;
    for(i=0; i<sizeof(nullgen.sval); i++)
        nullgen.sval[i] = 0;
    /*e: [[cinit()]] nullgen initialisation */
    /*s: [[cinit()]] hash initialisation from itab */
    for(i=0; i<NHASH; i++)
        hash[i] = S;
    for(i=0; itab[i].name; i++) {
        s = slookup(itab[i].name);
        s->type = itab[i].type;
        s->value = itab[i].value;
    }
    /*e: [[cinit()]] hash initialisation from itab */
    /*s: [[cinit()]] pathname initialisation from cwd */
    pathname = allocn(pathname, 0, 100);
    if(getwd(pathname, 99) == 0) {
        pathname = allocn(pathname, 100, 900);
        if(getwd(pathname, 999) == 0)
            strcpy(pathname, "/???");
    }
    /*e: [[cinit()]] pathname initialisation from cwd */
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

    outcode(AEND, Always, &nullgen, R_NONE, &nullgen);
    Bflush(&obuf);
}
/*e: function cclean(arm) */

/*s: function zname(arm) */
void
zname(char *n, int symkind, int symidx)
{

    Bputc(&obuf, ANAME);
    Bputc(&obuf, symkind);	/* type */
    Bputc(&obuf, symidx);	/* sym */
    while(*n) {
        Bputc(&obuf, *n);
        n++;
    }
    Bputc(&obuf, '\0');
}
/*e: function zname(arm) */

/*s: function zaddr(arm) */
void
zaddr(Gen *a, int symidx)
{
    /*s: [[zaddr()]] locals */
    long l;
    char *n;
    Ieee e;
    int i;
    /*e: [[zaddr()]] locals */

    // operand format: operand kind, register, symidx, symkind, optional offset
    Bputc(&obuf, a->type);
    Bputc(&obuf, a->reg);
    // idx in symbol table, 0 if no symbol involved in the operand
    Bputc(&obuf, symidx);
    // symkind of the symbol, if any
    Bputc(&obuf, a->symkind);

    switch(a->type) {
    /*s: [[zaddr()]] cases */
    case D_NONE:
        break;

    case D_REG:
    case D_PSR:
        break;

    case D_CONST:
    case D_OREG:
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
    /*x: [[zaddr()]] cases */
    case D_REGREG:
        Bputc(&obuf, a->offset);
        break;
    /*x: [[zaddr()]] cases */
    case D_FREG:
    case D_FPCR:
        break;
    /*x: [[zaddr()]] cases */
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
    /*e: [[zaddr()]] cases */
    default:
        print("unknown type %d\n", a->type);
        exits("arg");

    }
}
/*e: function zaddr(arm) */

/*s: global bcode(arm) */
static int bcode[] =
{
    ABEQ,
    ABNE,
    ABHS,
    ABLO,
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
outcode(int opcode, int scond,  Gen *g1, int reg, Gen *g2)
{
    /*s: [[outcode()]] locals */
    // symbol from, index in h[]
    int sf;
    // symbol to, index in h[]
    int st;
    // enum<sym_kind>
    int symkind;
    Sym *s;
    /*e: [[outcode()]] locals */

    /*s: [[outcode()]] adjust opcode and scond when opcode is AB */
    /* hack to make B.NE etc. work: turn it into the corresponding conditional*/
    if(opcode == AB){
        opcode = bcode[scond&0xf];
        scond = (scond & ~0xf) | Always;
    }
    /*e: [[outcode()]] adjust opcode and scond when opcode is AB */

    if(pass == 1)
        goto out;

    /*s: [[outcode()]] st and sf computation, and possible calls to zname */
    jackpot:
    sf = 0;
    s = g1->sym;

    while(s != S) {
        sf = s->symidx;

        if(sf < 0 || sf >= NSYM)
            sf = 0;

        symkind = g1->symkind;

        // already generated an ANAME for this symbol reference
        if(h[sf].symkind == symkind)
         if(h[sf].sym == s)
            break;

        s->symidx = symcounter;
        h[symcounter].sym = s;
        h[symcounter].symkind = symkind;
        sf = symcounter;
        zname(s->name, symkind, symcounter);

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

        symkind = g2->symkind;

        if(h[st].symkind == symkind)
          if(h[st].sym == s)
            break;

        s->symidx = symcounter;
        h[symcounter].sym = s;
        h[symcounter].symkind = symkind;
        st = symcounter;
        zname(s->name, symkind, symcounter);

        symcounter++;
        if(symcounter >= NSYM)
            symcounter = 1;

        if(st == sf)
            goto jackpot;
        break;
    }
    /*e: [[outcode()]] st and sf computation, and possible calls to zname */

    // Instruction serialized format: opcode, cond, optional reg, line, operands
    Bputc(&obuf, opcode);
    Bputc(&obuf, scond);
    Bputc(&obuf, reg);
    Bputc(&obuf, lineno);
    Bputc(&obuf, lineno>>8);
    Bputc(&obuf, lineno>>16);
    Bputc(&obuf, lineno>>24);
    zaddr(g1, sf);
    zaddr(g2, st);

out:
    if(opcode != AGLOBL && opcode != ADATA)
        pc++;
}
/*e: function outcode(arm) */

/*s: function outhist(arm) */
void
outhist(void)
{
    Gen g;
    Hist *h;
    char *p, *op;
    char *q;
    int n;

    g = nullgen;
    for(h = hist; h != H; h = h->link) {
        p = h->filename;

        /*s: [[outhist()]] adjust p and op if p is relative filename */
        op = nil;
        if(p && p[0] != '/' && h->local_line == 0 && pathname){
            if(pathname[0] == '/'){
                op = p;
                p = pathname;
            }
        }
        /*e: [[outhist()]] adjust p and op if p is relative filename */
        /*s: [[outhist()]] output each path component as an ANAME */
        while(p) {
            q = strchr(p, '/');
            if(q) {
                n = q-p;
                if(n == 0){
                    n = 1;	/* leading "/" */
                    *p = '/';
                }
                q++;
            } else {
                n = strlen(p);
                q = nil;
            }

            if(n) {
                Bputc(&obuf, ANAME);
                Bputc(&obuf, D_FILE);	/* type */ // symkind
                Bputc(&obuf, 1);	    /* sym */  // symidx
                Bputc(&obuf, '<');
                Bwrite(&obuf, p, n);
                Bputc(&obuf, '\0');
            }
            p = q;
            if(p == nil && op) {
                p = op;
                op = nil;
            }
        }
        /*e: [[outhist()]] output each path component as an ANAME */
        g.offset = h->local_line;

        Bputc(&obuf, AHISTORY);
        Bputc(&obuf, Always);
        Bputc(&obuf, 0); // reg, but could be R_NONE actually
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
/*s: function yylex */
long
yylex(void)
{
    int c;
    /*s: [[yylex()]] locals */
    int c1;
    /*x: [[yylex()]] locals */
    char *cp;
    Sym *s;
    /*e: [[yylex()]] locals */

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

    switch(c) {
    /*s: [[yylex()]] switch c cases */
    case '\n':
        lineno++;
        return ';';
    /*x: [[yylex()]] switch c cases */
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
    /*x: [[yylex()]] switch c cases */
    case '_':
    case '@':
    // case 'a'..'z' 'A'..'Z': (isalpha())
    talph:
        cp = symb;

    aloop:
        *cp++ = c;
        c = GETC();
        if(isalpha(c) || isdigit(c) || c == '_' || c == '$') // $
            goto aloop;

        // went too far, yyback(1)
        peekc = c;

        *cp = '\0';
        s = lookup();

        /*s: [[yylex()]] if macro symbol */
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
        /*e: [[yylex()]] if macro symbol */

        if(s->type == 0)
            s->type = LNAME;

        if(s->type == LNAME || s->type == LVAR || s->type == LLAB) {
            yylval.sym = s;
        } else {
            yylval.lval = s->value;
        }
        return s->type;
    /*x: [[yylex()]] switch c cases */
    // case '0'..'9': (isdigit())
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
    /*x: [[yylex()]] switch c cases */
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
    /*x: [[yylex()]] switch c cases */
    case '\'':
        c = escchar('\'');
        if(c == EOF)
            c = '\'';
        if(escchar('\'') != EOF)
            yyerror("missing '");
        yylval.lval = c;
        return LCONST;
    /*x: [[yylex()]] switch c cases */
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
    /*x: [[yylex()]] switch c cases */
    case '#':
        domacro();
        goto l0;
    /*e: [[yylex()]] switch c cases */
    default:
        return c;
    }
    peekc = c1;
    return c;
}
/*e: function yylex */

// #include "../cc/macbody"
/*e: assemblers/5a/lex.c */
