/*s: db/setup.c */

/*
 * init routines
 */
#include "defs.h"
#include "fns.h"

/*s: global symfil */
char	*symfil = nil;
/*e: global symfil */
/*s: global corfil */
char	*corfil = nil;
/*e: global corfil */

/*s: global dotmap */
Map	*dotmap;
/*e: global dotmap */

/*s: global fsym */
fdt fsym;
/*e: global fsym */
/*s: global fcor */
fdt fcor;
/*e: global fcor */
/*s: global fhdr (db/setup.c) */
static Fhdr fhdr;
/*e: global fhdr (db/setup.c) */

static int getfile(char*, int, int);

/*s: function setsym */
void
setsym(void)
{
    int ret;
    /*s: [[setsym()]] locals */
    Symbol s;
    /*e: [[setsym()]] locals */

    fsym = getfile(symfil, 1, wtflag);
    /*s: [[setsym()]] error managment on fsym */
    if(fsym < 0) {
        symmap = dumbmap(-1);
        return;
    }
    /*e: [[setsym()]] error managment on fsym */
    ret = crackhdr(fsym, &fhdr);
    if (ret) {
        machbytype(fhdr.type);
        symmap = loadmap(symmap, fsym, &fhdr);
        /*s: [[setsym()]] error managment on symmap */
        if (symmap == nil)
            symmap = dumbmap(fsym);
        /*e: [[setsym()]] error managment on symmap */
        ret = syminit(fsym, &fhdr);
        /*s: [[setsym()]] error managment on syminit */
        if (ret < 0)
            dprint("%r\n");
        /*e: [[setsym()]] error managment on syminit */

        /*s: [[setsym()]] if mach has sbreg */
        if (mach->sbreg && lookup(0, mach->sbreg, &s))
            mach->sb = s.value;
        /*e: [[setsym()]] if mach has sbreg */
    }
    /*s: [[setsym()]] error managment on crackhdr */
    else
        symmap = dumbmap(fsym);
    /*e: [[setsym()]] error managment on crackhdr */
}
/*e: function setsym */

/*s: function setcor */
void
setcor(void)
{
    int i;

    /*s: [[setcor()]] free previous cormap */
    if (cormap) {
        for (i = 0; i < cormap->nsegs; i++)
            if (cormap->seg[i].inuse)
                close(cormap->seg[i].fd);
    }
    /*e: [[setcor()]] free previous cormap */
    fcor = getfile(corfil, 2, ORDWR);
    /*s: [[setcor()]] error managment getfile */
    if (fcor <= 0) {
        if (cormap)
            free(cormap);
        cormap = dumbmap(-1);
        return;
    }
    /*e: [[setcor()]] error managment getfile */
    if(pid > 0) {	/* provide addressability to executing process */
        cormap = attachproc(pid, kflag, fcor, &fhdr);
        /*s: [[setcor()]] error managment cormap */
        if (!cormap)
            cormap = dumbmap(-1);
        /*e: [[setcor()]] error managment cormap */
    } else {
        cormap = newmap(cormap, 2);
        /*s: [[setcor()]] error managment cormap */
        if (!cormap)
            cormap = dumbmap(-1);
        /*e: [[setcor()]] error managment cormap */
        setmap(cormap, fcor, fhdr.txtaddr, fhdr.txtaddr+fhdr.txtsz, fhdr.txtaddr, "text");
        setmap(cormap, fcor, fhdr.dataddr, 0xffffffff, fhdr.dataddr, "data");
    }
    kmsys();
    return;
}
/*e: function setcor */

extern Mach mi386;
extern Machdata i386mach;

/*s: function dumbmap */
Map *
dumbmap(fdt fd)
{
    Map *dumb;


    dumb = newmap(0, 1);
    setmap(dumb, fd, 0, 0xffffffff, 0, "data");
    if (!mach) 			/* default machine = 386 */
        mach = &mi386;
    if (!machdata)
        machdata = &i386mach;
    return dumb;
}
/*e: function dumbmap */

/*s: function cmdmap */
/*
 * set up maps for a direct process image (/proc)
 */

void
cmdmap(Map *map)
{
    int i;
    char name[MAXSYM];

    extern char lastc;

    rdc();
    readsym(name);
    i = findseg(map, name);
    if (i < 0)	/* not found */
        error("Invalid map name");

    if (expr(0)) {
        if (strcmp(name, "text") == 0)
            textseg(expv, &fhdr);
        map->seg[i].b = expv;
    } else
        error("Invalid base address"); 
    if (expr(0))
        map->seg[i].e = expv;
    else
        error("Invalid end address"); 
    if (expr(0))
        map->seg[i].f = expv; 
    else
        error("Invalid file offset"); 
    if (rdc()=='?' && map == cormap) {
        if (fcor)
            close(fcor);
        fcor=fsym;
        corfil=symfil;
        cormap = symmap;
    } else if (lastc == '/' && map == symmap) {
        if (fsym)
            close(fsym);
        fsym=fcor;
        symfil=corfil;
        symmap=cormap;
    } else
        reread();
}
/*e: function cmdmap */

/*s: function getfile */
static fdt
getfile(char *filnam, int cnt, int omode)
{
    fdt f;

    if (filnam == nil)
        return ERROR_NEG1;

    if (strcmp(filnam, "-") == 0)
        return STDIN;
    f = open(filnam, omode|OCEXEC);

    /*s: [[getfile()]] error managment */
    if(f < 0 && omode == ORDWR){
        f = open(filnam, OREAD|OCEXEC);
        if(f >= 0)
            dprint("%s open read-only\n", filnam);
    }
    /*s: [[getfile()]] if wtflag */
    if (f < 0 && xargc > cnt && wtflag)
         f = create(filnam, 1, 0666);
    /*e: [[getfile()]] if wtflag */
    if (f < 0) {
        dprint("cannot open `%s': %r\n", filnam);
        return ERROR_NEG1;
    }
    /*e: [[getfile()]] error managment */
    return f;
}
/*e: function getfile */

/*s: function kmsys */
void
kmsys(void)
{
    int i;

    i = findseg(symmap, "text");
    if (i >= 0) {
        symmap->seg[i].b = symmap->seg[i].b & ~mach->ktmask;
        symmap->seg[i].e = ~0;
    }

    i = findseg(symmap, "data");
    if (i >= 0) {
        symmap->seg[i].b |= mach->kbase;
        symmap->seg[i].e |= mach->kbase;
    }
}
/*e: function kmsys */

/*s: function attachprocess */
void
attachprocess(void)
{
    char buf[100];
    Dir *sym, *mem;
    int fd;

    if (!adrflg) {
        dprint("used pid$a\n");
        return;
    }
    sym = dirfstat(fsym);
    sprint(buf, "/proc/%lud/mem", adrval);
    corfil = buf;
    setcor();
    sprint(buf, "/proc/%lud/text", adrval);
    fd = open(buf, OREAD);
    /*s: [[attachprocess()]] error managment */
    mem = nil;
    if (sym==nil || fd < 0 || (mem=dirfstat(fd))==nil
                 || sym->qid.path != mem->qid.path)
        dprint("warning: text images may be inconsistent\n");
    free(sym);
    free(mem);
    /*e: [[attachprocess()]] error managment */
    if (fd >= 0)
        close(fd);
}
/*e: function attachprocess */
/*e: db/setup.c */
