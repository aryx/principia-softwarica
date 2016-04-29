/*s: linkers/5l/asm.c */
#include	"l.h"
#include	"m.h"

/*s: constant Dbufslop */
#define	Dbufslop	100
/*e: constant Dbufslop */

/*s: function entryvalue(arm) */
long
entryvalue(void)
{
    char *a;
    Sym *s;

    a = INITENTRY; // usually "_main"
    /*s: [[entryvalue()]] if digit INITENTRY */
    if(*a >= '0' && *a <= '9')
        return atolwhex(a);
    /*e: [[entryvalue()]] if digit INITENTRY */

    s = lookup(a, 0);

    switch(s->type) {
    case SNONE:
        return INITTEXT; // no _main, start at beginning of binary then
    case STEXT:
        return s->value;
    /*s: [[entryvalue()]] if dynamic module case */
    case SDATA:
        if(dlm)
            return s->value+INITDAT;
    /*e: [[entryvalue()]] if dynamic module case */
    default:
        diag("entry not TEXT: %s", s->name);
        return 0;
    }
}
/*e: function entryvalue(arm) */



/*s: function strnput(arm) */
void
strnput(char *s, int n)
{
    for(; *s; s++){
        cput(*s);
        n--;
    }
    for(; n > 0; n--)
        cput(0);
}
/*e: function strnput(arm) */

/*s: function cput(arm) */
void
cput(int c)
{
    cbp[0] = c;
    cbp++;
    cbc--;
    if(cbc <= 0)
        cflush();
}
/*e: function cput(arm) */

/*s: function wput(arm) */
void
wput(long l)
{

    cbp[0] = l>>8;
    cbp[1] = l;
    cbp += 2;
    cbc -= 2;
    if(cbc <= 0)
        cflush();
}
/*e: function wput(arm) */

/*s: function wputl(arm) */
void
wputl(long l)
{

    cbp[0] = l;
    cbp[1] = l>>8;
    cbp += 2;
    cbc -= 2;
    if(cbc <= 0)
        cflush();
}
/*e: function wputl(arm) */

/*s: function lput(arm) */
void
lput(long l)
{

    cbp[0] = l>>24;
    cbp[1] = l>>16;
    cbp[2] = l>>8;
    cbp[3] = l;
    cbp += 4;
    cbc -= 4;
    if(cbc <= 0)
        cflush();
}
/*e: function lput(arm) */

/*s: function lputl(arm) */
void
lputl(long l)
{

    cbp[3] = l>>24;
    cbp[2] = l>>16;
    cbp[1] = l>>8;
    cbp[0] = l;
    cbp += 4;
    cbc -= 4;
    if(cbc <= 0)
        cflush();
}
/*e: function lputl(arm) */

/*s: function cflush */
void
cflush(void)
{
    int n;

    n = sizeof(buf.obuf) - cbc;
    if(n)
        write(cout, buf.obuf, n);

    cbp = buf.obuf;
    cbc = sizeof(buf.obuf);
}
/*e: function cflush */

/*s: function putsymb */
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
/*e: function putsymb */

/*s: function asmsym(arm) */
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
/*e: function asmsym(arm) */


/*s: constant MINLC(arm) */
#define	MINLC	4
/*e: constant MINLC(arm) */
/*s: function asmlc */
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
/*e: function asmlc */

/*s: function datblk(arm) */
void
datblk(long s, long n, bool sstring)
{
    Prog *p; 
    // absolute address of a DATA
    long a; // ulong?
    // size of a DATA
    int c; 
    // index in output buffer for a DATA
    long l;
    // index in value of a DATA
    int i;
    /*s: [[datblk()]] other locals */
    long j;
    /*x: [[datblk()]] other locals */
    char *cast;
    long d;
    /*x: [[datblk()]] other locals */
    Sym *v;
    /*x: [[datblk()]] other locals */
    long fl;
    /*e: [[datblk()]] other locals */

    memset(buf.dbuf, 0, n+Dbufslop);

    for(p = datap; p != P; p = p->link) {
        /*s: [[datblk()]] if sstring might continue */
        if(sstring != (p->from.sym->type == SSTRING))
            continue;
        /*e: [[datblk()]] if sstring might continue */
        // else
        curp = p;

        a = p->from.sym->value + p->from.offset;
        l = a - s;
        c = p->reg;

        i = 0;
        if(l < 0) {
            if(l+c <= 0)
                continue;
            while(l < 0) {
                l++;
                i++;
            }
        }
        if(l >= n)
            continue;
        // else

        /*s: [[datblk()]] sanity check multiple initialization */
        for(j=l+(c-i)-1; j>=l; j--)
            if(buf.dbuf[j]) {
                print("%P\n", p);
                diag("multiple initialization");
                break;
            }
        /*e: [[datblk()]] sanity check multiple initialization */

        switch(p->to.type) {
        /*s: [[datblk()]] switch type of destination cases */
        case D_SCONST:
            for(; i<c; i++) {
                buf.dbuf[l] = p->to.sval[i];
                l++;
            }
            break;
        /*x: [[datblk()]] switch type of destination cases */
        case D_CONST: case D_ADDR:
            d = p->to.offset;
            /*s: [[datblk()]] if D_ADDR case */
            v = p->to.sym;
            if(v) {
                switch(v->type) {
                /*s: [[datblk()]] in D_ADDR case, switch symbol type cases */
                case STEXT: case SSTRING:
                    d += p->to.sym->value;
                    break;
                case SDATA: case SBSS:
                    d += p->to.sym->value + INITDAT;
                    break;
                /*x: [[datblk()]] in D_ADDR case, switch symbol type cases */
                case SUNDEF:
                    ckoff(v, d);
                    d += p->to.sym->value;
                    break;
                /*e: [[datblk()]] in D_ADDR case, switch symbol type cases */
                }
                /*s: [[datblk()]] if dynamic module(arm) */
                if(dlm)
                    dynreloc(v, a+INITDAT, 1);
                /*e: [[datblk()]] if dynamic module(arm) */
            }
            /*e: [[datblk()]] if D_ADDR case */
            cast = (char*)&d;

            switch(c) {
            case 1:
                for(; i<c; i++) {
                    buf.dbuf[l] = cast[inuxi1[i]];
                    l++;
                }
                break;
            case 2:
                for(; i<c; i++) {
                    buf.dbuf[l] = cast[inuxi2[i]];
                    l++;
                }
                break;
            case 4:
                for(; i<c; i++) {
                    buf.dbuf[l] = cast[inuxi4[i]];
                    l++;
                }
                break;

            default:
                diag("bad nuxi %d %d%P", c, i, curp);
                break;
            }
            break;
        /*x: [[datblk()]] switch type of destination cases */
        case D_FCONST:
            switch(c) {
            default:
            case 4:
                fl = ieeedtof(p->to.ieee);
                cast = (char*)&fl;
                for(; i<c; i++) {
                    buf.dbuf[l] = cast[fnuxi4[i]];
                    l++;
                }
                break;
            case 8:
                cast = (char*)p->to.ieee;
                for(; i<c; i++) {
                    buf.dbuf[l] = cast[fnuxi8[i]];
                    l++;
                }
                break;
            }
            break;
        /*e: [[datblk()]] switch type of destination cases */
        default:
            diag("unknown mode in initialization%P", p);
            break;
        }
    }
    write(cout, buf.dbuf, n);
}
/*e: function datblk(arm) */



/*s: function asmb(arm) */
/// main -> <>
void
asmb(void)
{
    /*s: [[asmb()]] locals */
    long OFFSET;
    /*x: [[asmb()]] locals */
    Prog *p;
    Optab *o;
    /*x: [[asmb()]] locals */
    long t;
    /*x: [[asmb()]] locals */
    long etext;
    /*e: [[asmb()]] locals */

    DBG("%5.2f asm\n", cputime());

    // Text section
    /*s: [[asmb()]] Text section */
    OFFSET = HEADR;
    seek(cout, OFFSET, SEEK__START);

    pc = INITTEXT;
    for(p = firstp; p != P; p = p->link) {
        /*s: adjust curtext when iterate over instructions p */
        if(p->as == ATEXT)
            curtext = p;
        /*e: adjust curtext when iterate over instructions p */
        /*s: adjust autosize when iterate over instructions p */
        if(p->as == ATEXT) {
            autosize = p->to.offset + 4;
        }
        /*e: adjust autosize when iterate over instructions p */
        curp = p;
        /*s: [[asmb()]] in Text section generation, sanity check pc */
        if(p->pc != pc) {
            diag("phase error %lux sb %lux", p->pc, pc);
            if(!debug['a'])
                prasm(curp);
            pc = p->pc;
        }
        /*e: [[asmb()]] in Text section generation, sanity check pc */

        o = oplook(p);
        // generate ARM instruction(s)!
        asmout(p, o);

        pc += o->size;
    }
    /*s: [[asmb()]] before cflush, debug */
    if(debug['a']) {
        Bprint(&bso, "\n");
        Bflush(&bso);
    }
    /*e: [[asmb()]] before cflush, debug */
    cflush();

    /*s: [[asmb()]] Text section, output strings in text segment */
    /* output strings in text segment */
    etext = INITTEXT + textsize;
    for(t = pc; t < etext; t += sizeof(buf)-100) {
        if(etext-t > sizeof(buf)-100)
            datblk(t, sizeof(buf)-100, true);
        else
            datblk(t, etext-t, true);
    /*e: [[asmb()]] Text section, output strings in text segment */
    }
    /*e: [[asmb()]] Text section */

    // Data section
    /*s: [[asmb()]] Data section */
    curtext = P;
    switch(HEADTYPE) {
    /*s: [[asmb()]] switch HEADTYPE (to position after text) cases(arm) */
    case H_PLAN9:
        OFFSET = HEADR+textsize;
        seek(cout, OFFSET, SEEK__START);
        break;
    /*x: [[asmb()]] switch HEADTYPE (to position after text) cases(arm) */
    case H_ELF:
        OFFSET = HEADR+textsize;
        seek(cout, OFFSET, 0);
        break;
    /*e: [[asmb()]] switch HEADTYPE (to position after text) cases(arm) */
    }

    /*s: [[asmb()]] if dynamic module, before datblk() */
    if(dlm){
        char buf[8];

        write(cout, buf, INITDAT-textsize);
        textsize = INITDAT;
    }
    /*e: [[asmb()]] if dynamic module, before datblk() */

    for(t = 0; t < datsize; t += sizeof(buf)-100) {
        if(datsize-t > sizeof(buf)-100)
            datblk(t, sizeof(buf)-100, false);
        else
            datblk(t, datsize-t, false);
    }
    /*e: [[asmb()]] Data section */

    // Symbol and Line table sections
    /*s: [[asmb()]] symbol and line table sections */
    // modified by asmsym()
    symsize = 0;
    // modified by asmlc()
    lcsize = 0;

    if(!debug['s']) {
        switch(HEADTYPE) {
        /*s: [[asmb()]] switch HEADTYPE (for symbol table generation) cases(arm) */
        case H_PLAN9:
            OFFSET = HEADR+textsize+datsize;
            seek(cout, OFFSET, SEEK__START);
            break;
        /*x: [[asmb()]] switch HEADTYPE (for symbol table generation) cases(arm) */
        case H_ELF:
            break;
        /*e: [[asmb()]] switch HEADTYPE (for symbol table generation) cases(arm) */
        }
        DBG("%5.2f sym\n", cputime());
        asmsym();
        DBG("%5.2f pc\n", cputime());
        asmlc();

        /*s: [[asmb()]] if dynamic module, call asmdyn() */
        if(dlm)
            asmdyn();
        /*e: [[asmb()]] if dynamic module, call asmdyn() */
        cflush();
    }
    else {
        /*s: [[asmb()]] if dynamic module and no symbol table generation */
        if(dlm){
            seek(cout, HEADR+textsize+datsize, 0);
            asmdyn();
            cflush();
        }
        /*e: [[asmb()]] if dynamic module and no symbol table generation */
    }
    /*e: [[asmb()]] symbol and line table sections */

    // Header
    /*s: [[asmb()]] header section */
    DBG("%5.2f header\n", cputime());

    OFFSET = 0;
    seek(cout, OFFSET, SEEK__START);

    switch(HEADTYPE) {
    /*s: [[asmb()]] switch HEADTYPE (for header generation) cases(arm) */
    // see Exec in a.out.h
    case H_PLAN9:
        /*s: [[asmb()]] if dynamic module magic header adjustment(arm) */
        if(dlm)
            lput(0x80000000|0x647);	/* magic */
        /*e: [[asmb()]] if dynamic module magic header adjustment(arm) */
        else
            lput(0x647);			/* magic */

        lput(textsize);			/* sizes */
        lput(datsize);
        lput(bsssize);
        lput(symsize);			/* nsyms */

        lput(entryvalue());		/* va of entry */
        lput(0L);
        lput(lcsize);
        break;
    /*x: [[asmb()]] switch HEADTYPE (for header generation) cases(arm) */
    case H_ELF:
        debug['S'] = 1;			/* symbol table */
        elf32(ARM, ELFDATA2LSB, 0, nil);
        break;
    /*e: [[asmb()]] switch HEADTYPE (for header generation) cases(arm) */
    }
    /*e: [[asmb()]] header section */

    cflush();
}
/*e: function asmb(arm) */

/*e: linkers/5l/asm.c */
