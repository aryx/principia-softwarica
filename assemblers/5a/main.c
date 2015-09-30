/*s: assemblers/5a/main.c */
#include "a.h"

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

    if(*argv == '\0') {
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
    outhist(); // file history information at the beginning of the object

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

/*e: assemblers/5a/main.c */