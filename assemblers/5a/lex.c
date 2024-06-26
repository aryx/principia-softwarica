/*s: assemblers/5a/lex.c */
#include "a.h"
#include "y.tab.h"

/*s: struct [[Itab]](arm) */
struct Itab
{
    char	*name;

    //enum<token_code>
    ushort	type;
    //enum<Opcode> | enum<Register> | ... | int
    ushort	value;
};
/*e: struct [[Itab]](arm) */

/*s: global [[itab]](arm) */
// map<string, (token_code * token_value)>
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
    "MUL",		LARITH, AMUL,
    "DIV",		LARITH,	ADIV,
    "MOD",		LARITH,	AMOD,
    /*x: [[itab]] elements */
    "SLL",		LARITH,	ASLL,
    "SRL",		LARITH,	ASRL,
    "SRA",		LARITH,	ASRA,
    /*x: [[itab]] elements */
    "BIC",		LARITH,	ABIC,
    /*x: [[itab]] elements */
    "RSB",		LARITH,	ARSB,
    "ADC",		LARITH,	AADC,
    "SBC",		LARITH,	ASBC,
    "RSC",		LARITH,	ARSC,
    /*x: [[itab]] elements */
    "MVN",		LMVN, AMVN,	/* op2 ignored */
    /*x: [[itab]] elements */
    "MOVW",		LMOV, AMOVW,
    "MOVB",		LMOV, AMOVB,
    "MOVBU",	LMOV, AMOVBU,
    "MOVH",		LMOV, AMOVH,
    "MOVHU",	LMOV, AMOVHU,
    /*x: [[itab]] elements */
    "SWPW",		LSWAP, ASWPW,
    "SWPBU",	LSWAP, ASWPBU,
    /*x: [[itab]] elements */
    "B",		LBRANCH, AB,
    /*x: [[itab]] elements */
    "CMP",		LCMP,	ACMP,
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
    "TST",		LCMP,	ATST,
    "TEQ",		LCMP,	ATEQ,
    "CMN",		LCMP,	ACMN,
    /*x: [[itab]] elements */
    "BL",		LBRANCH, ABL,
    /*x: [[itab]] elements */
    "RET",		LRET, ARET,
    /*x: [[itab]] elements */
    "SWI",		LSWI, ASWI,
    /*x: [[itab]] elements */
    "RFE",		LRET, ARFE,
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
    "SB",		LSB,	N_EXTERN,
    "SP",		LSP,	N_LOCAL,
    "FP",		LFP,	N_PARAM,
    /*x: [[itab]] elements */
    "PC",		LPC,	D_BRANCH,
    /*x: [[itab]] elements */
    "TEXT",		LDEF, ATEXT,
    "GLOBL",	LDEF, AGLOBL,
    /*x: [[itab]] elements */
    "DATA",		LDATA, ADATA,
    /*x: [[itab]] elements */
    "WORD",		LWORD, AWORD,
    /*x: [[itab]] elements */
    "END",		LEND, AEND,
    /*x: [[itab]] elements */
    ".EQ",		LCOND,	0,
    ".NE",		LCOND,	1,
    ".HS",		LCOND,	2,
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
    ".S",		LS,	C_SBIT,
    /*x: [[itab]] elements */
    ".P",		LS,	C_PBIT,
    ".W",		LS,	C_WBIT,
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
    "SQRTF",	LSQRTFLOAT, ASQRTF,
    "SQRTD",	LSQRTFLOAT, ASQRTD,
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
    "MULA",		LMULA, AMULA,
    /*x: [[itab]] elements */
    "MULL",		LMULL, AMULL,
    "MULAL",	LMULL, AMULAL,
    "MULLU",	LMULL, AMULLU,
    "MULALU",	LMULL, AMULALU,
    /*x: [[itab]] elements */
    "MOVM",		LMOVM, AMOVM,
    /*x: [[itab]] elements */
    ".U",		LS,	C_UBIT,
    /*x: [[itab]] elements */
    ".IB",		LS,	C_PBIT|C_UBIT,
    ".IA",		LS,	C_UBIT,
    ".DB",		LS,	C_PBIT,
    ".DA",		LS,	0,
    /*x: [[itab]] elements */
    ".PW",		LS,	C_WBIT|C_PBIT,
    ".WP",		LS,	C_WBIT|C_PBIT,
    /*x: [[itab]] elements */
    ".IBW",		LS,	C_WBIT|C_PBIT|C_UBIT,
    ".IAW",		LS,	C_WBIT|C_UBIT,
    ".DBW",		LS,	C_WBIT|C_PBIT,
    ".DAW",		LS,	C_WBIT,
    /*x: [[itab]] elements */
    "CPSR",		LPSR,	0,
    "SPSR",		LPSR,	1,
    /*x: [[itab]] elements */
    ".F",		LS,	C_FBIT,
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
/*e: global [[itab]](arm) */

/*s: function [[cinit]](arm) */
/// main -> <>
void
cinit(void)
{
    /*s: [[cinit()]] locals */
    Sym *s;
    int i;
    /*e: [[cinit()]] locals */

    /*s: [[cinit()]] nullgen initialization */
    nullgen.type    = D_NONE; // no operand type set yet
    nullgen.reg     = R_NONE;
    nullgen.symkind = N_NONE;
    nullgen.sym = S;
    nullgen.offset = 0; // part of a union
    /*x: [[cinit()]] nullgen initialization */
    if(FPCHIP)
        nullgen.dval = 0;
    for(i=0; i<sizeof(nullgen.sval); i++)
        nullgen.sval[i] = 0;
    /*e: [[cinit()]] nullgen initialization */
    /*s: [[cinit()]] hash initialization from itab */
    for(i=0; i<NHASH; i++)
        hash[i] = S;
    for(i=0; itab[i].name; i++) {
        s = slookup(itab[i].name);
        s->value = itab[i].value;
        s->type = itab[i].type;
    }
    /*e: [[cinit()]] hash initialization from itab */
    /*s: [[cinit()]] pathname initialization from cwd */
    pathname = allocn(pathname, 0, 100);
    if(getwd(pathname, 99) == 0) {
        pathname = allocn(pathname, 100, 900);
        if(getwd(pathname, 999) == 0)
            strcpy(pathname, "/???");
    }
    /*e: [[cinit()]] pathname initialization from cwd */
}
/*e: function [[cinit]](arm) */

/*s: function [[syminit]] */
void
syminit(Sym *sym)
{
    sym->type = LNAME;
    sym->value = 0;
}
/*e: function [[syminit]] */

// now use aa.a8
//#include "../cc/lexbody"
//#include "../cc/compat"

// used to be in ../cc/lexbody and factorized between assemblers by
// using #include, but ugly, so I copy pasted the function for now
/*s: function [[yylex]] */
/// main -> assemble -> yyparse -> <>
long
yylex(void)
{
    int c;
    /*s: [[yylex()]] locals */
    int c1;
    /*x: [[yylex()]] locals */
    // ref<char> (target = symb)
    char *cp;
    // ref<Symbol> (owner = hash)
    Sym *s;
    /*x: [[yylex()]] locals */
    int baselog2;
    /*x: [[yylex()]] locals */
    int i;
    /*e: [[yylex()]] locals */

    /*s: [[yylex()]] peekc handling, starting part */
    c = peekc;
    if(c != IGN) {
        peekc = IGN; // consume the extra character saved in peekc
        goto l1; // skip the GETC(), we already have a character in c
    }
    /*e: [[yylex()]] peekc handling, starting part */
l0:
    c = GETC();
l1:
    if(c == EOF) {
        return EOF;
    }

    if(isspace(c)) {
        /*s: [[yylex()]] if c is newline */
        if(c == '\n') {
            lineno++;
            return ';'; // newline transformed in fake ';'
        }
        /*e: [[yylex()]] if c is newline */
        // ignore spaces
        goto l0;
    }
    // else

    /*s: [[yylex()]] before switch, if isxxx */
    if(isalpha(c))
        goto talph;
    /*x: [[yylex()]] before switch, if isxxx */
    if(isdigit(c))
        goto tnum;
    /*e: [[yylex()]] before switch, if isxxx */
    switch(c) {
//XxX: missing?
//    case '\n':
//        lineno++;
//        return ';';
    /*s: [[yylex()]] switch c cases */
    case '/':
        c1 = GETC();
        if(c1 == '/') {
            // '/''/' read; skip everything until next '\n'
            for(;;) {
                c = GETC();
                if(c == '\n')
                    goto l1; // which will convert the \n in c in a ';'
                if(c == EOF) {
                    yyerror("eof in comment");
                    errorexit();
                }
            }
        }
        if(c1 == '*') {
            // '/''*' read; skip everything until next '*''/'
            for(;;) {
                c = GETC();
                while(c == '*') { // not an if! to handle /** not finished */
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
    // XxX: case '.' too
    talph:
        cp = symb;

    aloop:
        *cp++ = c;
        c = GETC();
        if(isalpha(c) || isdigit(c) || c == '_' || c == '$')
            goto aloop;
        // went too far
        peekc = c;

        *cp = '\0';
        s = lookup(); // uses symb global (referenced by cp)
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
        //XxX?
        //if(s->type == 0)
        //    s->type = LNAME;

        /*s: [[yylex()]] in identifier case, set yylval */
        if(s->type == LNAME || s->type == LLAB || s->type == LVAR) {
            yylval.sym = s;
        } else {
            yylval.lval = s->value;
        }
        /*e: [[yylex()]] in identifier case, set yylval */
        return s->type;
    /*x: [[yylex()]] switch c cases */
    // case '0'..'9': (isdigit())
    tnum:
        cp = symb;
        if(c != '0')
            goto dc;
        // else, read a '0', maybe the start of an hexadecimal number
        /*s: [[yylex()]] in number case, 0xxx handling */
        *cp++ = c;
        c = GETC();
        baselog2 = 3; // 2^3, for octal
        if(c == 'x' || c == 'X') {
            baselog2 = 4; // 2^4, for hexadecimal
            c = GETC();
        } 
        else if(c < '0' || c > '7')
            goto dc;

        yylval.lval = 0;
        for(;;) {
            if(c >= '0' && c <= '9') {
                if(c > '7' && baselog2 == 3)
                    break;
                yylval.lval <<= baselog2;
                yylval.lval += c - '0';
                c = GETC();
                continue;
            }
            // else
            if(baselog2 == 3)
                break;
            // else
            /*s: [[yylex()]] in number case, 0xxx handling, normalize letters */
            if(c >= 'A' && c <= 'F')
                // c = lowercase(c)
                c += 'a' - 'A';
            /*e: [[yylex()]] in number case, 0xxx handling, normalize letters */
            if(c >= 'a' && c <= 'f') {
                yylval.lval <<= baselog2;
                yylval.lval += c - 'a' + 10;
                c = GETC();
                continue;
            }
            break;
        }
        //XxX: goto ncu;
        peekc = c;
        return LCONST;
        /*e: [[yylex()]] in number case, 0xxx handling */

    /*s: [[yylex()]] in number case, decimal dc label handling */
    dc:
        for(;;) {
            if(!isdigit(c))
                break;
            *cp++ = c;
            c = GETC();
        }
        /*s: [[yylex()]] in number case, in decimal case, float handling */
        if(c == '.')
            goto casedot;
        if(c == 'e' || c == 'E')
            goto casee;
         //XxX:         *cp = 0;
         //XxX:         if(sizeof(yylval.lval) == sizeof(vlong))
         //XxX:             yylval.lval = strtoll(symb, nil, 10);
         //XxX:         else
         //XxX:             yylval.lval = strtol(symb, nil, 10);
         //XxX: 
         //XxX:     ncu:
         //XxX:         while(c == 'U' || c == 'u' || c == 'l' || c == 'L')
         //XxX:             c = GETC();

        /*e: [[yylex()]] in number case, in decimal case, float handling */
        *cp = '\0';
        yylval.lval = strtol(symb, nil, 10);

        peekc = c;
        return LCONST;
    /*e: [[yylex()]] in number case, decimal dc label handling */
    /*s: [[yylex()]] in number case, float labels handling */
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
        *cp = '\0';
        peekc = c;
        if(FPCHIP) {
            yylval.dval = atof(symb);
            return LFCONST;
        } else {
            yyerror("assembler cannot interpret fp constants");
            yylval.lval = 1L;
            return LCONST;
        }
    /*e: [[yylex()]] in number case, float labels handling */
    /*x: [[yylex()]] switch c cases */
    case '.':
        c = GETC();
        if(isalpha(c)) { // an identifier
            cp = symb;
            *cp++ = '.';
            goto aloop;
        }
        if(isdigit(c)) { // a float
            cp = symb;
            *cp++ = '.';
            goto casedot;
        }
        // else
        peekc = c;
        return '.'; // a single '.'
    /*x: [[yylex()]] switch c cases */
    case '\'':
        c = escchar('\'');
        /*s: [[yylex()]] in character case, if c is EOF */
        if(c == EOF)
            c = '\'';
        /*e: [[yylex()]] in character case, if c is EOF */
        if(escchar('\'') != EOF)
            yyerror("missing '");

        yylval.lval = c;
        return LCONST;
    /*x: [[yylex()]] switch c cases */
    case '"':
        memcpy(yylval.sval, nullgen.sval, sizeof(yylval.sval));
        cp = yylval.sval;
        i = 0;
        for(;;) {
            c = escchar('"');
            if(c == EOF)
                break;
            if(i < sizeof(yylval.sval))
                *cp++ = c;
            i++;
        }
        if(i > sizeof(yylval.sval))
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
    /*s: [[yylex()]] peekc handling, ending part */
    peekc = c1;
    /*e: [[yylex()]] peekc handling, ending part */
    return c;
}
/*e: function [[yylex]] */

// #include "../cc/macbody"
/*e: assemblers/5a/lex.c */
