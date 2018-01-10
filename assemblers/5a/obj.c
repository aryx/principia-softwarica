/*s: assemblers/5a/obj.c */
#include "a.h"

/*s: function [[zname]](arm) */
/// outcode -> <>
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
/*e: function [[zname]](arm) */

/*s: function [[zaddr]](arm) */
/// main -> assemble -> yyparse -> outcode -> <>
void
outopd(Gen *a, int symidx)
{
    /*s: [[zaddr()]] locals(arm) */
    long l;
    char *n;
    Ieee e;
    int i;
    /*e: [[zaddr()]] locals(arm) */

    Bputc(&obuf, a->type);
    Bputc(&obuf, a->reg);
    // idx in symbol table, 0 if no symbol involved in the operand
    Bputc(&obuf, symidx);
    // symkind of the symbol, or N_NONE
    Bputc(&obuf, a->symkind);

    switch(a->type) {
    /*s: [[zaddr()]] cases(arm) */
    case D_NONE:
        break;

    case D_REG:
        break;

    case D_CONST:
    case D_ADDR:
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
    /*x: [[zaddr()]] cases(arm) */
    case D_FREG:
    case D_FPCR:
        break;
    /*x: [[zaddr()]] cases(arm) */
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
    /*x: [[zaddr()]] cases(arm) */
    case D_REGREG:
        Bputc(&obuf, a->offset);
        break;
    /*x: [[zaddr()]] cases(arm) */
    case D_PSR:
        break;
    /*e: [[zaddr()]] cases(arm) */
    default:
        print("unknown type %d\n", a->type);
        exits("arg");

    }
}
/*e: function [[zaddr]](arm) */

/*s: global [[bcode]](arm) */
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
/*e: global [[bcode]](arm) */

/*s: function [[symidx_of_symopt]] */
int
symidx_of_symopt(Sym *sym, int symkind)
{
    int idx = 0;
   
    if(sym != S) {
        idx = sym->symidx;
        /*s: [[symidx_of_symopt()]] sanity check idx */
        if(idx < 0 || idx >= NSYM)
            idx = 0;
        /*e: [[symidx_of_symopt()]] sanity check idx */
        
        // already generated an ANAME for this symbol reference?
        if((h[idx].symkind != symkind || h[idx].sym != sym)) {
            sym->symidx = symcounter;
            h[symcounter].sym = sym;
            h[symcounter].symkind = symkind;
            idx = symcounter;
            zname(sym->name, symkind, symcounter);
            
            symcounter++;
            if(symcounter >= NSYM)
                // circular array
                symcounter = 1;
        }
    }
    return idx;
}
/*e: function [[symidx_of_symopt]] */

/*s: function [[outcode]](arm) */
/// main -> assemble -> yyparse -> <>
void
outcode(int opcode, int scond,  Gen *g1, int reg, Gen *g2)
{
    /*s: [[outcode()]] locals */
    // symbol from, index in h[]
    int sf;
    // symbol to, index in h[]
    int st;
    int oldsymcounter;
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
    oldsymcounter = symcounter;
    sf = symidx_of_symopt(g1->sym, g1->symkind);
    st = symidx_of_symopt(g2->sym, g2->symkind);
    /*s: [[outcode()]] if jackpot condition goto jackpot */
    if (sf == st && sf != 0 && symcounter != oldsymcounter) 
       goto jackpot;
    /*e: [[outcode()]] if jackpot condition goto jackpot */
    /*e: [[outcode()]] st and sf computation, and possible calls to zname */

    Bputc(&obuf, opcode);
    Bputc(&obuf, scond);
    Bputc(&obuf, reg);
    Bputc(&obuf, lineno);
    Bputc(&obuf, lineno>>8);
    Bputc(&obuf, lineno>>16);
    Bputc(&obuf, lineno>>24);
    outopd(g1, sf);
    outopd(g2, st);

out:
    if(opcode != AGLOBL && opcode != ADATA)
        pc++;
}
/*e: function [[outcode]](arm) */

/*s: function [[outhist]](arm) */
/// main -> assemble -> <> (at beginning of pass 2)
void
outhist(void)
{
    Gen g;
    Hist *h;
    char *p;
    /*s: [[outhist()]] locals(arm) */
    char *q;
    int n;
    /*x: [[outhist()]] locals(arm) */
    char *op;
    /*e: [[outhist()]] locals(arm) */

    g = nullgen;
    for(h = hist; h != H; h = h->link) {
        p = h->filename;

        /*s: [[outhist()]] adjust p and op if p is relative filename */
        if(p && p[0] != '/' && h->local_line == 0 && pathname && pathname[0] == '/') {
            op = p; // save p
            p = pathname; // start with cwd
        } else {
            op = nil; // start directly with p
        }
        /*e: [[outhist()]] adjust p and op if p is relative filename */
        /*s: [[outhist()]] output each path component as an ANAME */
        // =~ split("/", p) ...
        while(p) {
            q = strchr(p, '/');
            if(q) {
                n = q-p;
                if(n == 0){
                    n = 1;	/* leading "/" */
                    *p = '/'; // redundant?
                }
                q++;
            } else {
                n = strlen(p);
                q = nil;
            }

            if(n) {
                Bputc(&obuf, ANAME);
                Bputc(&obuf, N_FILE);	/* type */ // symkind
                Bputc(&obuf, 1);	    /* sym */  // symidx
                Bputc(&obuf, '<');
                Bwrite(&obuf, p, n);
                Bputc(&obuf, '\0');
            }
            p = q;
            /*s: [[outhist()]] adjust p and op if p was a relative filename */
            if(p == nil && op) {
                p = op;
                op = nil;
            }
            /*e: [[outhist()]] adjust p and op if p was a relative filename */
        }
        /*e: [[outhist()]] output each path component as an ANAME */

        Bputc(&obuf, AHISTORY);
        Bputc(&obuf, Always);
        Bputc(&obuf, 0); // reg, but could be R_NONE too 
        Bputc(&obuf, h->global_line);
        Bputc(&obuf, h->global_line>>8);
        Bputc(&obuf, h->global_line>>16);
        Bputc(&obuf, h->global_line>>24);
        outopd(&nullgen, 0);
        g.offset = h->local_line;
        outopd(&g, 0);
    }
}
/*e: function [[outhist]](arm) */

/*e: assemblers/5a/obj.c */
