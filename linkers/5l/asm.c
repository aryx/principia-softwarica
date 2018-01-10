/*s: linkers/5l/asm.c */
#include	"l.h"
#include	"m.h"

/*s: function [[entryvalue]](arm) */
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
        // could warn no _main found?
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
/*e: function [[entryvalue]](arm) */

/*s: function [[asmb]](arm) */
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
/*e: function [[asmb]](arm) */

/*e: linkers/5l/asm.c */
