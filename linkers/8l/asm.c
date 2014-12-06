/*s: linkers/8l/asm.c */
#include	"l.h"

/*s: constant Dbufslop */
#define	Dbufslop	100
/*e: constant Dbufslop */

/*s: global spsize(x86) */
long	spsize = 0;
/*e: global spsize(x86) */

void	datblk(long, long);

/*s: function entryvalue(x86) */
long
entryvalue(void)
{
    char *a;
    Sym *s;

    a = INITENTRY;

    /*s: [[entryvalue()]] if digit INITENTRY */
    if(*a >= '0' && *a <= '9')
        return atolwhex(a);
    /*e: [[entryvalue()]] if digit INITENTRY */

    s = lookup(a, 0);
    // no _main found, maybe pure asm, start at beginning of TEXT section (va)
    if(s->type == 0)
        return INITTEXT;

    switch(s->type) {
    case STEXT:
        return s->value;
    /*s: [[entryvalue()]] if dynamic module case */
    case SDATA:
        if(dlm)
            return s->value+INITDAT;
    /*e: [[entryvalue()]] if dynamic module case */
    default:
        diag("entry not text: %s", s->name);
    }
}
/*e: function entryvalue(x86) */

/*s: function wputl(x86) */
/* these need to take long arguments to be compatible with elf.c */
void
wputl(long w)
{
    cput(w);
    cput(w>>8);
}
/*e: function wputl(x86) */

/*s: function wput(x86) */
void
wput(long w)
{
    cput(w>>8);
    cput(w);
}
/*e: function wput(x86) */

/*s: function lput(x86) */
void
lput(long l)
{
    cput(l>>24);
    cput(l>>16);
    cput(l>>8);
    cput(l);
}
/*e: function lput(x86) */

/*s: function lputl(x86) */
void
lputl(long l)
{
    cput(l);
    cput(l>>8);
    cput(l>>16);
    cput(l>>24);
}
/*e: function lputl(x86) */

/*s: function llput */
void
llput(vlong v)
{
    lput(v>>32);
    lput(v);
}
/*e: function llput */

/*s: function llputl */
void
llputl(vlong v)
{
    lputl(v);
    lputl(v>>32);
}
/*e: function llputl */

/*s: function strnput(x86) */
void
strnput(char *s, int n)
{
    for(; *s && n > 0; s++){
        cput(*s);
        n--;
    }
    while(n > 0){
        cput(0);
        n--;
    }
}
/*e: function strnput(x86) */

/*s: function asmb(x86) */
void
asmb(void)
{
    Prog *p;
    long v, magic;
    int a;
    byte *op1;

    DBG("%5.2f asmb\n", cputime());

    // TEXT SECTION

    seek(cout, HEADR, SEEK__START);
    pc = INITTEXT;
    curp = firstp;

    for(p = firstp; p != P; p = p->link) {
        if(p->as == ATEXT)
            curtext = p;
        if(p->pc != pc) {
            if(!debug['a'])
                print("%P\n", curp);
            diag("phase error %lux sb %lux in %s", p->pc, pc, TNAME);
            pc = p->pc;
        }
        curp = p;

        // generate instruction!
        asmins(p);

        if(cbc < sizeof(and))
            cflush();
        a = (andptr - and);

        if(debug['a']) {
            Bprint(&bso, pcstr, pc);
            for(op1 = and; op1 < andptr; op1++)
                Bprint(&bso, "%.2ux", *op1 & 0xff);
            Bprint(&bso, "\t%P\n", curp);
        }

        /*s: [[asmb()]] if dynamic module, when iterate from firstp(x86) */
        if(dlm) {
            if(p->as == ATEXT)
                reloca = nil;
            else if(reloca != nil)
                diag("reloc failure: %P", curp);
        }
        /*e: [[asmb()]] if dynamic module, when iterate from firstp(x86) */

        memmove(cbp, and, a);
        cbp += a;
        pc += a;
        cbc -= a;
    }
    cflush();

    // DATA SECTION

    switch(HEADTYPE) {
    case H_PLAN9:
        seek(cout, HEADR+textsize, SEEK__START);
        break;
    /*s: [[asmb()]] switch HEADTYPE (to position after text) cases(x86) */
    case H_GARBAGE:
        seek(cout, rnd(HEADR+textsize, 8192), 0);
        break;
    case H_COFF:
        textsize = rnd(HEADR+textsize, 4096)-HEADR;
        seek(cout, textsize+HEADR, 0);
        break;
    case H_ELF: // like H_PLAN9
        seek(cout, HEADR+textsize, 0);
        break;
    case H_COM:
    case H_EXE:
        seek(cout, HEADR+rnd(textsize, INITRND), 0);
        break;
    /*e: [[asmb()]] switch HEADTYPE (to position after text) cases(x86) */
    default:
        diag("unknown header type %ld", HEADTYPE);
    }

    DBG("%5.2f datblk\n", cputime());

    /*s: [[asmb()]] if dynamic module, before datblk()(x86) */
    if(dlm){
        char buf[8];

        write(cout, buf, INITDAT-textsize);
        textsize = INITDAT;
    }
    /*e: [[asmb()]] if dynamic module, before datblk()(x86) */

    for(v = 0; v < datsize; v += sizeof(buf)-Dbufslop) {
        if(datsize-v > sizeof(buf)-Dbufslop)
            datblk(v, sizeof(buf)-Dbufslop);
        else
            datblk(v, datsize-v);
    }

    // SYMBOL TABLE

    // modified by asmsym()
    symsize = 0;
    // modified by asmlc()
    lcsize = 0;

    if(!debug['s']) {
        DBG("%5.2f sym\n", cputime());

        switch(HEADTYPE) {
        case H_PLAN9:
            seek(cout, HEADR+textsize+datsize, 0);
            break;
        /*s: [[asmb()]] switch HEADTYPE (for symbol table generation) cases(x86) */
        case H_GARBAGE:
        case H_COFF:
            seek(cout, rnd(HEADR+textsize, INITRND)+datsize, 0);
            break;
        //case H_PLAN9:
        case H_ELF:
            seek(cout, HEADR+textsize+datsize, 0);
            break;
        case H_COM:
        case H_EXE:
            debug['s'] = 1;
            break;
        /*e: [[asmb()]] switch HEADTYPE (for symbol table generation) cases(x86) */
        default:
            seek(cout, rnd(HEADR+textsize, 8192)+datsize, 0);
            break;
        }

        asmsym();

        DBG("%5.2f sp\n", cputime());
        DBG("%5.2f pc\n", cputime());

        asmlc();

        /*s: [[asmb()]] if dynamic module, call asmdyn()(x86) */
        if(dlm)
            asmdyn();
        /*e: [[asmb()]] if dynamic module, call asmdyn()(x86) */
        cflush();
    } else {
        /*s: [[asmb()]] if dynamic module and no symbol table generation(x86) */
        if(dlm){
            seek(cout, HEADR+textsize+datsize, 0);
            asmdyn();
            cflush();
        }
        /*e: [[asmb()]] if dynamic module and no symbol table generation(x86) */
    }

    // HEADER

    DBG("%5.2f headr\n", cputime());

    seek(cout, 0L, SEEK__START);

    switch(HEADTYPE) {
    // see Exec in a.out.h
    case H_PLAN9:	/* plan9 */
        magic = 4*11*11+7;
        /*s: [[asmb()]] if dynamic module magic header adjustment(x86) */
        if(dlm)
            magic |= 0x80000000;
        /*e: [[asmb()]] if dynamic module magic header adjustment(x86) */
        lput(magic);			/* magic */
        lput(textsize);			/* sizes */
        lput(datsize);
        lput(bsssize);
        lput(symsize);			/* nsyms */

        lput(entryvalue());		/* va of entry */

        lput(spsize);			/* sp offsets */
        lput(lcsize);			/* line offsets */
        break;
    /*s: [[asmb()]] switch HEADTYPE (for header generation) cases(x86) */
    default:
    case H_GARBAGE:	/* garbage */
        lput(0x160L<<16);		/* magic and sections */
        lput(0L);			/* time and date */
        lput(rnd(HEADR+textsize, 4096)+datsize);
        lput(symsize);			/* nsyms */
        lput((0x38L<<16)|7L);		/* size of optional hdr and flags */
        lput((0413<<16)|0437L);		/* magic and version */
        lput(rnd(HEADR+textsize, 4096));/* sizes */
        lput(datsize);
        lput(bsssize);
        lput(entryvalue());		/* va of entry */
        lput(INITTEXT-HEADR);		/* va of base of text */
        lput(INITDAT);			/* va of base of data */
        lput(INITDAT+datsize);		/* va of base of bss */
        lput(~0L);			/* gp reg mask */
        lput(0L);
        lput(0L);
        lput(0L);
        lput(0L);
        lput(~0L);			/* gp value ?? */
        break;
    case H_COFF:	/* unix coff */
        /*
         * file header
         */
        lputl(0x0004014c);		/* 4 sections, magic */
        lputl(0);			/* unix time stamp */
        lputl(0);			/* symbol table */
        lputl(0);			/* nsyms */
        lputl(0x0003001c);		/* flags, sizeof a.out header */
        /*
         * a.out header
         */
        lputl(0x10b);			/* magic, version stamp */
        lputl(rnd(textsize, INITRND));	/* text sizes */
        lputl(datsize);			/* data sizes */
        lputl(bsssize);			/* bss sizes */
        lput(entryvalue());		/* va of entry */
        lputl(INITTEXT);		/* text start */
        lputl(INITDAT);			/* data start */
        /*
         * text section header
         */
        strnput(".text", 8);
        lputl(HEADR);			/* pa */
        lputl(HEADR);			/* va */
        lputl(textsize);		/* text size */
        lputl(HEADR);			/* file offset */
        lputl(0);			/* relocation */
        lputl(0);			/* line numbers */
        lputl(0);			/* relocation, line numbers */
        lputl(0x20);			/* flags text only */
        /*
         * data section header
         */
        strnput(".data", 8);
        lputl(INITDAT);			/* pa */
        lputl(INITDAT);			/* va */
        lputl(datsize);			/* data size */
        lputl(HEADR+textsize);		/* file offset */
        lputl(0);			/* relocation */
        lputl(0);			/* line numbers */
        lputl(0);			/* relocation, line numbers */
        lputl(0x40);			/* flags data only */
        /*
         * bss section header
         */
        strnput(".bss", 8);
        lputl(INITDAT+datsize);		/* pa */
        lputl(INITDAT+datsize);		/* va */
        lputl(bsssize);			/* bss size */
        lputl(0);			/* file offset */
        lputl(0);			/* relocation */
        lputl(0);			/* line numbers */
        lputl(0);			/* relocation, line numbers */
        lputl(0x80);			/* flags bss only */
        /*
         * comment section header
         */
        strnput(".comment", 8);
        lputl(0);			/* pa */
        lputl(0);			/* va */
        lputl(symsize+lcsize);		/* comment size */
        lputl(HEADR+textsize+datsize);	/* file offset */
        lputl(HEADR+textsize+datsize);	/* offset of syms */
        lputl(HEADR+textsize+datsize+symsize);/* offset of line numbers */
        lputl(0);			/* relocation, line numbers */
        lputl(0x200);			/* flags comment only */
        break;
    case H_COM:
        /* MS-DOS .COM */
        break;
    case H_EXE:
        /* fake MS-DOS .EXE */
        v = rnd(HEADR+textsize, INITRND)+datsize;
        wputl(0x5A4D);			/* 'MZ' */
        wputl(v % 512);			/* bytes in last page */
        wputl(rnd(v, 512)/512);		/* total number of pages */
        wputl(0x0000);			/* number of reloc items */
        v = rnd(HEADR-(INITTEXT & 0xFFFF), 16);
        wputl(v/16);			/* size of header */
        wputl(0x0000);			/* minimum allocation */
        wputl(0xFFFF);			/* maximum allocation */
        wputl(0x0000);			/* initial ss value */
        wputl(0x0100);			/* initial sp value */
        wputl(0x0000);			/* complemented checksum */
        v = entryvalue();
        wputl(v);			/* initial ip value (!) */
        wputl(0x0000);			/* initial cs value */
        wputl(0x0000);
        wputl(0x0000);
        wputl(0x003E);			/* reloc table offset */
        wputl(0x0000);			/* overlay number */
        break;
    case H_ELF:
        elf32(I386, ELFDATA2LSB, 0, nil);
        break;
    /*e: [[asmb()]] switch HEADTYPE (for header generation) cases(x86) */
    }

    cflush();
}
/*e: function asmb(x86) */

/*s: function cflush */
void
cflush(void)
{
    int n;

    n = sizeof(buf.cbuf) - cbc;
    if(n)
        write(cout, buf.cbuf, n);
    cbp = buf.cbuf;
    cbc = sizeof(buf.cbuf);
}
/*e: function cflush */

/*s: function datblk(x86) */
void
datblk(long s, long n)
{
    Prog *p;
    char *cast;
    long l, fl, j;
    int i, c;

    memset(buf.dbuf, 0, n+Dbufslop);
    for(p = datap; p != P; p = p->link) {
        curp = p;
        l = p->from.sym->value + p->from.offset - s;
        c = p->from.scale;
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
        if(p->as != AINIT && p->as != ADYNT) {
            for(j=l+(c-i)-1; j>=l; j--)
                if(buf.dbuf[j]) {
                    print("%P\n", p);
                    diag("multiple initialization");
                    break;
                }
        }
        switch(p->to.type) {
        case D_FCONST:
            switch(c) {
            default:
            case 4:
                fl = ieeedtof(&p->to.ieee);
                cast = (char*)&fl;
                if(debug['a'] && i == 0) {
                    Bprint(&bso, pcstr, l+s+INITDAT);
                    for(j=0; j<c; j++)
                        Bprint(&bso, "%.2ux", cast[fnuxi4[j]] & 0xff);
                    Bprint(&bso, "\t%P\n", curp);
                }
                for(; i<c; i++) {
                    buf.dbuf[l] = cast[fnuxi4[i]];
                    l++;
                }
                break;
            case 8:
                cast = (char*)&p->to.ieee;
                if(debug['a'] && i == 0) {
                    Bprint(&bso, pcstr, l+s+INITDAT);
                    for(j=0; j<c; j++)
                        Bprint(&bso, "%.2ux", cast[fnuxi8[j]] & 0xff);
                    Bprint(&bso, "\t%P\n", curp);
                }
                for(; i<c; i++) {
                    buf.dbuf[l] = cast[fnuxi8[i]];
                    l++;
                }
                break;
            }
            break;

        case D_SCONST:
            if(debug['a'] && i == 0) {
                Bprint(&bso, pcstr, l+s+INITDAT);
                for(j=0; j<c; j++)
                    Bprint(&bso, "%.2ux", p->to.scon[j] & 0xff);
                Bprint(&bso, "\t%P\n", curp);
            }
            for(; i<c; i++) {
                buf.dbuf[l] = p->to.scon[i];
                l++;
            }
            break;

        default:
            fl = p->to.offset;
            if(p->to.type == D_ADDR) {
                if(p->to.index != D_STATIC && p->to.index != D_EXTERN)
                    diag("DADDR type%P", p);
                if(p->to.sym) {
                    if(p->to.sym->type == SUNDEF)
                        ckoff(p->to.sym, fl);
                    fl += p->to.sym->value;
                    if(p->to.sym->type != STEXT && p->to.sym->type != SUNDEF)
                        fl += INITDAT;
                    /*s: [[datblk()]] if dynamic module(x86) */
                    if(dlm)
                        dynreloc(p->to.sym, l+s+INITDAT, 1);
                    /*e: [[datblk()]] if dynamic module(x86) */
                }
            }
            cast = (char*)&fl;
            switch(c) {
            default:
                diag("bad nuxi %d %d\n%P", c, i, curp);
                break;
            case 1:
                if(debug['a'] && i == 0) {
                    Bprint(&bso, pcstr, l+s+INITDAT);
                    for(j=0; j<c; j++)
                        Bprint(&bso, "%.2ux", cast[inuxi1[j]] & 0xff);
                    Bprint(&bso, "\t%P\n", curp);
                }
                for(; i<c; i++) {
                    buf.dbuf[l] = cast[inuxi1[i]];
                    l++;
                }
                break;
            case 2:
                if(debug['a'] && i == 0) {
                    Bprint(&bso, pcstr, l+s+INITDAT);
                    for(j=0; j<c; j++)
                        Bprint(&bso, "%.2ux", cast[inuxi2[j]] & 0xff);
                    Bprint(&bso, "\t%P\n", curp);
                }
                for(; i<c; i++) {
                    buf.dbuf[l] = cast[inuxi2[i]];
                    l++;
                }
                break;
            case 4:
                if(debug['a'] && i == 0) {
                    Bprint(&bso, pcstr, l+s+INITDAT);
                    for(j=0; j<c; j++)
                        Bprint(&bso, "%.2ux", cast[inuxi4[j]] & 0xff);
                    Bprint(&bso, "\t%P\n", curp);
                }
                for(; i<c; i++) {
                    buf.dbuf[l] = cast[inuxi4[i]];
                    l++;
                }
                break;
            }
            break;
        }
    }
    write(cout, buf.dbuf, n);
}
/*e: function datblk(x86) */

/*s: function rnd */
long
rnd(long v, long r)
{
    long c;

    if(r <= 0)
        return v;
    v += r - 1;
    c = v % r;
    if(c < 0)
        c += r;
    v -= c;
    return v;
}
/*e: function rnd */
/*e: linkers/8l/asm.c */
