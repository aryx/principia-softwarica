/*s: linkers/5l/debugging.c */
#include	"l.h"

/*s: function [[putsymb]] */
void
putsymb(char *s, int t, long v, int ver)
{
    int i, f;

    /*s: [[putsymb()]] adjust string [[s]] if file symbol */
    if(t == 'f')
        s++;
    /*e: [[putsymb()]] adjust string [[s]] if file symbol */

    // value
    lput(v);
    // type
    if(ver)
        t += 'a' - 'A'; // lowercase(t)
    cput(t+0x80);			/* 0x80 is variable length */

    /*s: [[putsymb()]] if z or Z */
    if(t == 'z' || t == 'Z') {
        cput(s[0]);
        for(i=1; s[i] != '\0' || s[i+1] != '\0'; i += 2) {
            cput(s[i]);
            cput(s[i+1]);
        }
        cput('\0');
        cput('\0');
        i++;
    }
    /*e: [[putsymb()]] if z or Z */
    else {
        // name
        for(i=0; s[i]; i++)
            cput(s[i]);
        // end marker
        cput('\0');
    }
    symsize += 4 + 1 + i + 1;

    /*s: [[putsymb()]] debug */
    if(debug['n']) {
        /*s: [[putsymb()]] if z or Z in debug output */
        if(t == 'z' || t == 'Z') {
            Bprint(&bso, "%c %.8lux ", t, v);
            for(i=1; s[i] != 0 || s[i+1] != 0; i+=2) {
                f = ((s[i]&0xff) << 8) | (s[i+1]&0xff);
                Bprint(&bso, "/%x", f);
            }
            Bprint(&bso, "\n");
            return;
        }
        /*e: [[putsymb()]] if z or Z in debug output */
        if(ver)
            Bprint(&bso, "%c %.8lux %s<%d>\n", t, v, s, ver);
        else
            Bprint(&bso, "%c %.8lux %s\n", t, v, s);
    }
    /*e: [[putsymb()]] debug */
}
/*e: function [[putsymb]] */

/*s: function [[asmsym]](arm) */
/// main -> asmb -> <>
void
asmsym(void)
{
    Sym *s;
    int h;
    Prog *p;
    /*s: [[asmsym()]] other locals */
    Auto *a;
    /*e: [[asmsym()]] other locals */

    /*s: [[asmsym()]] generate symbol for etext */
    s = lookup("etext", 0);
    if(s->type == STEXT)
        putsymb(s->name, 'T', s->value, s->version);
    /*e: [[asmsym()]] generate symbol for etext */

    // data symbols
    for(h=0; h<NHASH; h++)
        for(s=hash[h]; s!=S; s=s->link)
            switch(s->type) {
            case SDATA:
                putsymb(s->name, 'D', s->value+INITDAT, s->version);
                continue;
            case SBSS:
                putsymb(s->name, 'B', s->value+INITDAT, s->version);
                continue;
            /*s: [[asmsym()]] in symbol table iteration, switch section cases */
            case SFILE:
                putsymb(s->name, 'f', s->value, s->version);
                continue;
            /*x: [[asmsym()]] in symbol table iteration, switch section cases */
            case SSTRING:
                putsymb(s->name, 'T', s->value, s->version);
                continue;
            /*e: [[asmsym()]] in symbol table iteration, switch section cases */
            }

    // procedure symbols
    for(p=textp; p!=P; p=p->cond) {
        s = p->from.sym;
        if(s->type == STEXT) {
            /* filenames first */
            /*s: [[asmsym()]] call putsymb for filenames */
            for(a=p->to.autom; a; a=a->link)
                if(a->type == N_FILE)
                    putsymb(a->asym->name, 'z', a->aoffset, 0);
                else
                if(a->type == N_LINE)
                    putsymb(a->asym->name, 'Z', a->aoffset, 0);
            /*e: [[asmsym()]] call putsymb for filenames */
            
            if(p->mark & LEAF)
                putsymb(s->name, 'L', s->value, s->version);
            else
                putsymb(s->name, 'T', s->value, s->version);

            // local symbols
            /*s: [[asmsym()]] frame symbols */
            /* frame, auto and param after */
            putsymb(".frame", 'm', p->to.offset+4, 0);
            for(a=p->to.autom; a; a=a->link)
                if(a->type == N_LOCAL)
                    putsymb(a->asym->name, 'a', -a->aoffset, 0);
                else
                if(a->type == N_PARAM)
                    putsymb(a->asym->name, 'p', a->aoffset, 0);
            /*e: [[asmsym()]] frame symbols */
        }
    }
    if(debug['v'] || debug['n']) {
        Bprint(&bso, "symsize = %lud\n", symsize);
        Bflush(&bso);
    }
}
/*e: function [[asmsym]](arm) */


/*s: constant [[MINLC]](arm) */
#define	MINLC	4
/*e: constant [[MINLC]](arm) */
/*s: function [[asmlc]] */
void
asmlc(void)
{
    long oldpc, oldlc;
    Prog *p;
    long v;
    long s;

    oldpc = INITTEXT;
    oldlc = 0;
    for(p = firstp; p != P; p = p->link) {
        if(p->line == oldlc || p->as == ATEXT || p->as == ANOP) {
            /*s: adjust curtext when iterate over instructions p */
            if(p->as == ATEXT)
                curtext = p;
            /*e: adjust curtext when iterate over instructions p */
            /*s: [[asmlc()]] dump instruction p, debug */
            if(debug['V'])
                Bprint(&bso, "%6lux %P\n", p->pc, p);
            /*e: [[asmlc()]] dump instruction p, debug */
            continue;
        }
        // else
        /*s: [[asmlc()]] dump lcsize, debug */
        if(debug['V'])
            Bprint(&bso, "\t\t%6ld", lcsize);
        /*e: [[asmlc()]] dump lcsize, debug */
        v = (p->pc - oldpc) / MINLC;
        while(v) {
            s = (v < 127)? v : 127; // min(), but impossible more than 6 (o6)
            cput(s+128);	/* 129-255 +pc */
            /*s: [[asmlc()]] dump s, debug */
            if(debug['V'])
                Bprint(&bso, " pc+%ld*%d(%ld)", s, MINLC, s+128);
            /*e: [[asmlc()]] dump s, debug */
            v -= s;
            lcsize++;
        }
        s = p->line - oldlc;
        oldlc = p->line;
        oldpc = p->pc + MINLC;

        if(s > 64 || s < -64) {
            cput(0);	/* 0 vv +lc */
            cput(s>>24);
            cput(s>>16);
            cput(s>>8);
            cput(s);
            /*s: [[asmlc()]] dump big line change, debug */
            if(debug['V']) {
                if(s > 0)
                    Bprint(&bso, " lc+%ld(%d,%ld)\n",
                        s, 0, s);
                else
                    Bprint(&bso, " lc%ld(%d,%ld)\n",
                        s, 0, s);
                Bprint(&bso, "%6lux %P\n",
                    p->pc, p);
            }
            /*e: [[asmlc()]] dump big line change, debug */
            lcsize += 5;
        } else {
            if(s > 0) {
                cput(0+s);	/* 1-64 +lc */
                /*s: [[asmlc()]] dump small line increment, debug */
                if(debug['V']) {
                    Bprint(&bso, " lc+%ld(%ld)\n", s, 0+s);
                    Bprint(&bso, "%6lux %P\n",
                        p->pc, p);
                }
                /*e: [[asmlc()]] dump small line increment, debug */
            } else {
                cput(64-s);	/* 65-128 -lc */
                /*s: [[asmlc()]] dump negative line increment, debug */
                if(debug['V']) {
                    Bprint(&bso, " lc%ld(%ld)\n", s, 64-s);
                    Bprint(&bso, "%6lux %P\n",
                        p->pc, p);
                }
                /*e: [[asmlc()]] dump negative line increment, debug */
            }
            lcsize++;
        }
    }
    // padding
    while(lcsize & 1) {
        s = 129;
        cput(s);
        lcsize++;
    }
    if(debug['v'] || debug['V'])
        Bprint(&bso, "lcsize = %ld\n", lcsize);
    Bflush(&bso);
}
/*e: function [[asmlc]] */

/*e: linkers/5l/debugging.c */
