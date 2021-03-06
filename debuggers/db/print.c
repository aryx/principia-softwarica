/*s: db/print.c */
/*
 *
 *	debugger
 *
 */
#include "defs.h"
#include "fns.h"

extern	int	infile;
extern	int	outfile;
extern	int	maxpos;

/* general printing routines ($) */

/*s: global [[Ipath]] */
char	*Ipath = INCDIR;
/*e: global [[Ipath]] */
/*s: global [[tracetype]] */
static	int	tracetype;
/*e: global [[tracetype]] */
static void	printfp(Map*, int);

/*s: function [[ptrace]] */
/*
 *	callback on stack trace
 */
static void
ptrace(Map *map, uvlong pc, uvlong sp, Symbol *sym)
{
    char buf[512];

    USED(map);
    dprint("%s(", sym->name);
    printparams(sym, sp);
    dprint(") ");
    printsource(sym->value);
    dprint(" called from ");
    symoff(buf, 512, pc, CTEXT);
    dprint("%s ", buf);
    printsource(pc);
    dprint("\n");

    if(tracetype == 'C')
        printlocals(sym, sp);
}
/*e: function [[ptrace]] */

/*s: function [[printtrace]] */
void
printtrace(int modif)
{
    /*s: [[printtrace()]] locals */
    int i;
    ulong w;
    BKPT *bk;
    Symbol s;
    int stack;
    char *fname;
    char buf[512];
    /*x: [[printtrace()]] locals */
    uvlong pc, sp, link;
    /*e: [[printtrace()]] locals */

    if (cntflg==FALSE)
        cntval = -1;

    switch (modif) {
    /*s: [[printtrace()]] switch modif cases */
    case 0:
    case '?':
        if (pid)
            dprint("pid = %d\n",pid);
        else
            prints("no process\n");
        flushbuf();
    /*x: [[printtrace()]] switch modif cases */
    case 'm':
        printmap("? map", symmap);
        printmap("/ map", cormap);
        break;
    /*x: [[printtrace()]] switch modif cases */
    case 'S':
        printsym();
        break;
    /*x: [[printtrace()]] switch modif cases */
    /*print externals*/
    case 'e':
        for (i = 0; globalsym(&s, i); i++) {
            if (get4(cormap, s.value, &w) > 0)
                dprint("%s/%12t%#lux\n", s.name, w);
        }
        break;
    /*x: [[printtrace()]] switch modif cases */
    case 'c':
    case 'C':
        tracetype = modif;
        if (machdata->ctrace) {
            if (adrflg) {
                /*
                 * trace from jmpbuf for multi-threaded code.
                 * assume sp and pc are in adjacent locations
                 * and mach->szaddr in size.
                 */
                if (geta(cormap, adrval, &sp) < 0 ||
                    geta(cormap, adrval+mach->szaddr, &pc) < 0)
                        error("%r");
            } else {
                sp = rget(cormap, mach->sp);
                pc = rget(cormap, mach->pc);
            }
            if(mach->link)
                link = rget(cormap, mach->link);
            else
                link = 0;
            if (machdata->ctrace(cormap, pc, sp, link, ptrace) <= 0)
                error("no stack frame");
        }
        break;
    /*x: [[printtrace()]] switch modif cases */
    case 'r':
    case 'R':
        printregs(modif);
        return;
    /*x: [[printtrace()]] switch modif cases */
    case 'f':
    case 'F':
        printfp(cormap, modif);
        return;
    /*x: [[printtrace()]] switch modif cases */
    case 'q':
    case 'Q':
        done();
    /*x: [[printtrace()]] switch modif cases */
    case 'a':
        attachprocess();
        break;
    /*x: [[printtrace()]] switch modif cases */
    case 's':
        maxoff=(adrflg?adrval:MAXOFF);
        break;
    /*x: [[printtrace()]] switch modif cases */
    case 'M':
        fname = getfname();
        if (machbyname(fname) == 0)
            dprint("unknown name\n");;
        break;
    /*x: [[printtrace()]] switch modif cases */
    case 'w':
        maxpos=(adrflg?adrval:MAXPOS);
        break;
    /*x: [[printtrace()]] switch modif cases */
    /*print breakpoints*/
    case 'b':
    case 'B':
        for (bk=bkpthead; bk; bk=bk->nxtbkpt)
            if (bk->flag) {
                symoff(buf, 512, (WORD)bk->loc, CTEXT);
                dprint(buf);
                if (bk->count != 1)
                    dprint(",%d", bk->count);
                dprint(":%c %s", 
                         bk->flag == BKPTTMP ? 'B' : 'b', 
                         bk->comm);
            }
        break;
    /*x: [[printtrace()]] switch modif cases */
    case '<':
        if (cntval == 0) {
            while (readchar() != EOR)
                ;
            reread();
            break;
        }
        if (rdc() == '<')
            stack = 1;
        else {
            stack = 0;
            reread();
        }
        fname = getfname();
        redirin(stack, fname);
        break;
    /*x: [[printtrace()]] switch modif cases */
    case '>':
        fname = getfname();
        redirout(fname);
        break;
    /*x: [[printtrace()]] switch modif cases */
    case 'k':
        kmsys();
        break;
    /*e: [[printtrace()]] switch modif cases */
    default:
        error("bad `$' command");
    }

}
/*e: function [[printtrace]] */

/*s: function [[getfname]] */
char *
getfname(void)
{
    static char fname[ARB];
    char *p;

    if (rdc() == EOR) {
        reread();
        return (0);
    }
    p = fname;
    do {
        *p++ = lastc;
        if (p >= &fname[ARB-1])
            error("filename too long");
    } while (rdc() != EOR);
    *p = 0;
    reread();
    return (fname);
}
/*e: function [[getfname]] */

/*s: function [[printfp]] */
static void
printfp(Map *map, int modif)
{
    Reglist *rp;
    int i;
    int ret;
    char buf[512];

    for (i = 0, rp = mach->reglist; rp->rname; rp += ret) {
        ret = 1;
        if (!(rp->rflags & RFLT))
            continue;
        ret = fpformat(map, rp, buf, sizeof(buf), modif);
        if (ret < 0) {
            werrstr("Register %s: %r", rp->rname);
            error("%r");
        }
            /* double column print */
        if (i&0x01)
            dprint("%40t%-8s%-12s\n", rp->rname, buf);
        else
            dprint("\t%-8s%-12s", rp->rname, buf);
        i++;
    }
}
/*e: function [[printfp]] */

/*s: function [[redirin]] */
void
redirin(int stack, char *file)
{
    char *pfile;

    if (file == 0) {
        iclose(-1, 0);
        return;
    }
    iclose(stack, 0);
    if ((infile = open(file, 0)) < 0) {
        pfile = smprint("%s/%s", Ipath, file);
        infile = open(pfile, 0);
        free(pfile);
        if(infile < 0) {
            infile = STDIN;
            error("cannot open");
        }
    }
}
/*e: function [[redirin]] */

/*s: function [[printmap]] */
void
printmap(char *s, Map *map)
{
    int i;

    if (!map)
        return;
    if (map == symmap)
        dprint("%s%12t`%s'\n", s, fsym < 0 ? "-" : symfil);
    else if (map == cormap)
        dprint("%s%12t`%s'\n", s, fcor < 0 ? "-" : corfil);
    else
        dprint("%s\n", s);

    for (i = 0; i < map->nsegs; i++) {
        if (map->seg[i].inuse)
            dprint("%s%8t%-16#llux %-16#llux %-16#llux\n",
                map->seg[i].name, 
                map->seg[i].b,
                map->seg[i].e, 
                map->seg[i].f);
    }
}
/*e: function [[printmap]] */

/*s: function [[printsym]] */
/*
 *	dump the raw symbol table
 */
void
printsym(void)
{
    int i;
    Sym *sp;

    for (i = 0; sp = getsym(i); i++) {
        switch(sp->type) {
        case 't':
        case 'l':
            dprint("%16#llux t %s\n", sp->value, sp->name);
            break;
        case 'T':
        case 'L':
            dprint("%16#llux T %s\n", sp->value, sp->name);
            break;
        case 'D':
        case 'd':
        case 'B':
        case 'b':
        case 'a':
        case 'p':
        case 'm':
            dprint("%16#llux %c %s\n", sp->value, sp->type, sp->name);
            break;
        default:
            break;
        }
    }
}
/*e: function [[printsym]] */

/*s: constant [[STRINGSZ]] */
#define	STRINGSZ	128
/*e: constant [[STRINGSZ]] */

/*s: function [[printsource]] */
/*
 *	print the value of dot as file:line
 */
void
printsource(ADDR dot)
{
    char str[STRINGSZ];

    if (fileline(str, STRINGSZ, dot))
        dprint("%s", str);
}
/*e: function [[printsource]] */

/*s: function [[printpc]] */
void
printpc(void)
{
    char buf[512];

    dot = rget(cormap, mach->pc);
    if(dot){
        printsource((long)dot);
        printc(' ');
        symoff(buf, sizeof(buf), (long)dot, CTEXT);
        dprint("%s/", buf);
        if (machdata->das(cormap, dot, 'i', buf, sizeof(buf)) < 0)
            error("%r");
        dprint("%16t%s\n", buf);
    }
}
/*e: function [[printpc]] */

/*s: function [[printlocals]] */
void
printlocals(Symbol *fn, ADDR fp)
{
    int i;
    ulong w;
    Symbol s;

    s = *fn;
    for (i = 0; localsym(&s, i); i++) {
        if (s.class != CAUTO)
            continue;
        if (get4(cormap, fp-s.value, &w) > 0)
            dprint("%8t%s.%s/%10t%#lux\n", fn->name, s.name, w);
        else
            dprint("%8t%s.%s/%10t?\n", fn->name, s.name);
    }
}
/*e: function [[printlocals]] */

/*s: function [[printparams]] */
void
printparams(Symbol *fn, ADDR fp)
{
    int i;
    Symbol s;
    ulong w;
    int first = 0;

    fp += mach->szaddr;			/* skip saved pc */
    s = *fn;
    for (i = 0; localsym(&s, i); i++) {
        if (s.class != CPARAM)
            continue;
        if (first++)
            dprint(", ");
        if (get4(cormap, fp+s.value, &w) > 0)
            dprint("%s=%#lux", s.name, w);
    }
}
/*e: function [[printparams]] */
/*e: db/print.c */
