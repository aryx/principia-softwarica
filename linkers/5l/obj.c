/*s: linkers/5l/obj.c */
#include	"l.h"
#include	<ar.h>

#ifndef	DEFAULT
/*s: constant DEFAULT */
#define	DEFAULT	'9'
/*e: constant DEFAULT */
#endif

/*s: global noname linker */
char	*noname		= "<none>";
/*e: global noname linker */
/*s: global symname linker */
char	symname[]	= SYMDEF;
/*e: global symname linker */
/*s: global thechar */
char	thechar;
/*e: global thechar */
/*s: global thestring */
char	*thestring;
/*e: global thestring */

/*s: global libdir */
// growing_array<dirname>
char**	libdir;
/*e: global libdir */
/*s: global nlibdir */
// index of next free entry in libdir
int	nlibdir	= 0;
/*e: global nlibdir */
/*s: global maxlibdir */
// index of last free entry in libdir
static	int	maxlibdir = 0;
/*e: global maxlibdir */

/*s: function usage, linker */
void
usage(void)
{
    diag("usage: %s [-options] objects", argv0);
    errorexit();
}
/*e: function usage, linker */

/*s: function isobjfile */
static int
isobjfile(char *f)
{
    int n, v;
    Biobuf *b;
    char buf1[5], buf2[SARMAG];

    b = Bopen(f, OREAD);
    if(b == nil)
        return 0;
    n = Bread(b, buf1, 5);
    if(n == 5 && (buf1[2] == 1 && buf1[3] == '<' || buf1[3] == 1 && buf1[4] == '<'))
        v = 1;	/* good enough for our purposes */
    else{
        Bseek(b, 0, 0);
        n = Bread(b, buf2, SARMAG);
        v = n == SARMAG && strncmp(buf2, ARMAG, SARMAG) == 0;
    }
    Bterm(b);
    return v;
}
/*e: function isobjfile */

/*s: function main(arm) */
void
main(int argc, char *argv[])
{
    /*s: [[main()]] locals(arm) */
    bool load_libs;
    /*x: [[main()]] locals(arm) */
    char *root;
    /*x: [[main()]] locals(arm) */
    int c;
    char name[LIBNAMELEN];
    char *a;
    /*e: [[main()]] locals(arm) */

    thechar = '5';
    thestring = "arm";   
    outfile = "5.out";

    /*s: [[main()]] debug initialization(arm) */
    Binit(&bso, 1, OWRITE);
    listinit(); // fmtinstall()
    /*e: [[main()]] debug initialization(arm) */

    ARGBEGIN {
    /*s: [[main()]] command line processing(arm) */
    case 'o':
        outfile = ARGF();
        break;
    /*x: [[main()]] command line processing(arm) */
    case 'H':
        a = ARGF();
        if(a)
            HEADTYPE = atolwhex(a);
        /* do something about setting INITTEXT */
        break;
    case 'T':
        a = ARGF();
        if(a)
            INITTEXT = atolwhex(a);
        break;
    case 'D':
        a = ARGF();
        if(a)
            INITDAT = atolwhex(a);
        break;
    case 'E':
        a = ARGF();
        if(a)
            INITENTRY = a;
        break;
    case 'R':
        a = ARGF();
        if(a)
            INITRND = atolwhex(a);
        break;
    /*x: [[main()]] command line processing(arm) */
    case 'L':
        addlibpath(EARGF(usage()));
        break;
    /*x: [[main()]] command line processing(arm) */
    case 'x':	/* produce export table */
        doexp = true;
        if(argv[1] != nil && argv[1][0] != '-' && !isobjfile(argv[1]))
            readundefs(ARGF(), SEXPORT);
        break;
    /*x: [[main()]] command line processing(arm) */
    case 'u':	/* produce dynamically loadable module */
        dlm = true;
        if(argv[1] != nil && argv[1][0] != '-' && !isobjfile(argv[1]))
            readundefs(ARGF(), SIMPORT);
        break;
    /*x: [[main()]] command line processing(arm) */
    case 'P':
        a = ARGF();
        if(a)
            INITTEXTP = atolwhex(a);
        break;
    /*x: [[main()]] command line processing(arm) */
    default:
        c = ARGC();
        if(c >= 0 && c < sizeof(debug))
            debug[c]++;
        break;
    /*e: [[main()]] command line processing(arm) */
    } ARGEND

    USED(argc);
    if(*argv == nil)
        usage();

    /*s: [[main()]] addlibpath("/{thestring}/lib") or ccroot */
    /*s: [[main()]] change root if ccroot */
    root = getenv("ccroot");

    if(root != nil && *root != '\0') {
        if(!fileexists(root)) {
            diag("nonexistent $ccroot: %s", root);
            errorexit();
        }
    }else
        root = "";
    /*e: [[main()]] change root if ccroot */

    // usually /{thestring}/lib/ as root = ""
    snprint(name, sizeof(name), "%s/%s/lib", root, thestring);
    addlibpath(name);
    /*e: [[main()]] addlibpath("/{thestring}/lib") or ccroot */
    /*s: [[main()]] set HEADTYPE, INITTEXT, INITDAT, etc */
    /*s: [[main()]] adjust HEADTYPE if debug flags(arm) */
    if(!debug['9'] && !debug['U'] && !debug['B'])
        debug[DEFAULT] = true;

    if(HEADTYPE == -1) {
        if(debug['U'])
            HEADTYPE = 0;
        if(debug['B'])
            HEADTYPE = 1;
        if(debug['9'])
            HEADTYPE = 2;
    }
    /*e: [[main()]] adjust HEADTYPE if debug flags(arm) */
    switch(HEADTYPE) {
    /*s: [[main()]] switch HEADTYPE cases(arm) */
    case H_PLAN9:
        HEADR = 32L;
        if(INITTEXT == -1)
            INITTEXT = 4096+32;
        if(INITDAT == -1)
            INITDAT = 0;
        if(INITRND == -1)
            INITRND = 4096;
        break;
    /*x: [[main()]] switch HEADTYPE cases(arm) */
    case 0:	/* no header */
    case 6:	/* no header, padded segments */
        HEADR = 0L;
        if(INITTEXT == -1)
            INITTEXT = 0;
        if(INITDAT == -1)
            INITDAT = 0;
        if(INITRND == -1)
            INITRND = 4;
        break;
    case 1:	/* aif for risc os */
        HEADR = 128L;
        if(INITTEXT == -1)
            INITTEXT = 0x10005000 + HEADR;
        if(INITDAT == -1)
            INITDAT = 0;
        if(INITRND == -1)
            INITRND = 4;
        break;
    case 3:	/* boot for NetBSD */
        HEADR = 32L;
        if(INITTEXT == -1)
            INITTEXT = 0xF0000020L;
        if(INITDAT == -1)
            INITDAT = 0;
        if(INITRND == -1)
            INITRND = 4096;
        break;
    case 4: /* boot for IXP1200 */
        HEADR = 0L;
        if(INITTEXT == -1)
            INITTEXT = 0x0;
        if(INITDAT == -1)
            INITDAT = 0;
        if(INITRND == -1)
            INITRND = 4;
        break;
    case 5: /* boot for ipaq */
        HEADR = 16L;
        if(INITTEXT == -1)
            INITTEXT = 0xC0008010;
        if(INITDAT == -1)
            INITDAT = 0;
        if(INITRND == -1)
            INITRND = 1024;
        break;
    case 7:	/* elf executable */
        HEADR = rnd(Ehdr32sz+3*Phdr32sz, 16);
        if(INITTEXT == -1)
            INITTEXT = 4096+HEADR;
        if(INITDAT == -1)
            INITDAT = 0;
        if(INITRND == -1)
            INITRND = 4;
        break;
    /*e: [[main()]] switch HEADTYPE cases(arm) */
    default:
        diag("unknown -H option");
        errorexit();
    }
    /*s: [[main()]] last INITXXX adjustments */
    if(INITDAT != 0 && INITRND != 0)
        print("warning: -D0x%lux is ignored because of -R0x%lux\n",
            INITDAT, INITRND);
    /*x: [[main()]] last INITXXX adjustments */
    if (INITTEXTP == -1)
        INITTEXTP = INITTEXT;
    /*e: [[main()]] last INITXXX adjustments */
    DBG("HEADER = -H0x%d -T0x%lux -D0x%lux -R0x%lux\n",
            HEADTYPE, INITTEXT, INITDAT, INITRND);
    /*e: [[main()]] set HEADTYPE, INITTEXT, INITDAT, etc */
    /*s: [[main()]] initialize globals(arm) */
    /*s: [[main()]] set zprg(arm) */
    zprg.as = AGOK;
    zprg.scond = COND_ALWAYS; 
    zprg.reg = R_NONE;
    zprg.from.type = D_NONE;
    zprg.from.symkind = N_NONE;
    zprg.from.reg = R_NONE;
    zprg.to = zprg.from;
    /*e: [[main()]] set zprg(arm) */
    /*x: [[main()]] initialize globals(arm) */
    load_libs = !debug['l'];
    /*x: [[main()]] initialize globals(arm) */
    buildop(); // ???
    /*x: [[main()]] initialize globals(arm) */
    nuxiinit(); // ???
    /*x: [[main()]] initialize globals(arm) */
    cbp = buf.cbuf;
    cbc = sizeof(buf.cbuf);
    /*e: [[main()]] initialize globals(arm) */

    cout = create(outfile, 1, 0775);
    if(cout < 0) {
        diag("cannot create %s: %r", outfile);
        errorexit();
    }

    // ------ main functions  ------
    /*s: [[main()]] cout is ready, LET'S GO(arm) */
    firstp = prg();
    lastp = firstp;

    /*s: [[main()]] set INITENTRY */
    if(INITENTRY == nil) {
        INITENTRY = "_main";
        /*s: [[main()]] adjust INITENTRY if profiling */
        if(debug['p'])
            INITENTRY = "_mainp";
        /*e: [[main()]] adjust INITENTRY if profiling */
        if(load_libs)
            lookup(INITENTRY, 0)->type = SXREF;
    } else {
        /*s: [[main()]] if digit INITENTRY */
        if(!(*INITENTRY >= '0' && *INITENTRY <= '9'))
           lookup(INITENTRY, 0)->type = SXREF;
        /*e: [[main()]] if digit INITENTRY */
    }
    /*e: [[main()]] set INITENTRY */

    // Loading
    while(*argv)
        objfile(*argv++);
    if(load_libs)
        loadlib();

    firstp = firstp->link;
    if(firstp == P)
        goto out;

    // Resolving
    /*s: [[main()]] resolving phases */
    /*s: [[main()]] if export table or dynamic module(arm) */
    if(doexp || dlm){
        EXPTAB = "_exporttab";
        zerosig(EXPTAB);
        zerosig("etext");
        zerosig("edata");
        zerosig("end");

        /*s: [[main()]] if dynamic module(arm) */
        if(dlm){
            initdiv();
            import();
            HEADTYPE = H_PLAN9;
            INITTEXT = INITDAT = 0;
            INITRND = 8;
            INITENTRY = EXPTAB;
        }
        /*e: [[main()]] if dynamic module(arm) */
        else
            divsig();

        export();
    }
    /*e: [[main()]] if export table or dynamic module(arm) */
    patch();
    /*s: [[main()]] call doprofxxx() if profiling */
    if(debug['p'])
        if(debug['1'])
            doprof1();
        else
            doprof2();
    /*e: [[main()]] call doprofxxx() if profiling */
    dodata();
    follow();
    if(firstp == P)
        goto out;
    noops();
    span();
    /*e: [[main()]] resolving phases */

    // Generating (writing to cout, finally)
    asmb();

    // Checking
    undef();
    /*e: [[main()]] cout is ready, LET'S GO(arm) */

out:
    /*s: [[main()]] profile report */
    if(debug['v']) {
        Bprint(&bso, "%5.2f cpu time\n", cputime());
        Bprint(&bso, "%ld symbols\n", nsymbol);
        Bprint(&bso, "%ld memory used\n", thunk);
        Bprint(&bso, "%d sizeof adr\n", sizeof(Adr));
        Bprint(&bso, "%d sizeof prog\n", sizeof(Prog));
        Bflush(&bso);
    }
    /*e: [[main()]] profile report */
    errorexit();
}
/*e: function main(arm) */

/*s: function addlibpath */
void
addlibpath(char *arg)
{
    char **p;

    // growing array libdir
    if(nlibdir >= maxlibdir) {
        if(maxlibdir == 0)
            maxlibdir = 8;
        else
            maxlibdir *= 2;
        p = malloc(maxlibdir*sizeof(*p));
        if(p == nil) {
            diag("out of memory");
            errorexit();
        }
        memmove(p, libdir, nlibdir*sizeof(*p));
        free(libdir);
        libdir = p;
    }

    libdir[nlibdir++] = strdup(arg);
}
/*e: function addlibpath */

/*s: function findlib */
char*
findlib(char *file)
{
    int i;
    char name[LIBNAMELEN];

    for(i = 0; i < nlibdir; i++) {
        snprint(name, sizeof(name), "%s/%s", libdir[i], file);
        if(fileexists(name))
            return libdir[i];
    }
    return nil;
}
/*e: function findlib */

/*s: function loadlib */
void
loadlib(void)
{
    int i;
    long h;
    Sym *s;

loop:
    xrefresolv = false;

    for(i=0; i<libraryp; i++) {
        DBG("%5.2f autolib: %s (from %s)\n", cputime(), library[i], libraryobj[i]);
        objfile(library[i]);
    }
    /*s: [[loadlib()]] if xrefresolv */
    if(xrefresolv)
        for(h=0; h<nelem(hash); h++)
             for(s = hash[h]; s != S; s = s->link)
                 if(s->type == SXREF)
                     goto loop;
    /*e: [[loadlib()]] if xrefresolv */
}
/*e: function loadlib */

/*s: function objfile */
void
objfile(char *file)
{
    fdt f;
    long l;
    char magbuf[SARMAG];
    /*s: [[objfile()]] other locals */
    long off, esym, cnt;
    bool work;
    Sym *s;
    char pname[LIBNAMELEN];
    char name[LIBNAMELEN];
    struct ar_hdr arhdr;
    char *e, *start, *stop;
    /*e: [[objfile()]] other locals */

    DBG("%5.2f ldobj: %s\n", cputime(), file);

    /*s: [[objfile()]] adjust file if -lxxx filename */
    if(file[0] == '-' && file[1] == 'l') {
        snprint(pname, sizeof(pname), "lib%s.a", file+2);
        e = findlib(pname);
        if(e == nil) {
            diag("cannot find library: %s", file);
            errorexit();
        }
        snprint(name, sizeof(name), "%s/%s", e, pname);
        file = name;
    }
    /*e: [[objfile()]] adjust file if -lxxx filename */

    f = open(file, 0);
    if(f < 0) {
        diag("cannot open %s: %r", file);
        errorexit();
    }

    l = read(f, magbuf, SARMAG);

    // is it a regular object (and not a library)
    if(l != SARMAG || strncmp(magbuf, ARMAG, SARMAG)){
        /* load it as a regular file */
        l = seek(f, 0L, SEEK__END);
        seek(f, 0L, SEEK__START);

        // the important call!
        ldobj(f, l, file);

        close(f);
        return;
    }

    /*s: [[objfile()]] when file is a library */
        DBG("%5.2f ldlib: %s\n", cputime(), file);
        l = read(f, &arhdr, SAR_HDR);
        if(l != SAR_HDR) {
            diag("%s: short read on archive file symbol header", file);
            goto out;
        }
        if(strncmp(arhdr.name, symname, strlen(symname))) {
            diag("%s: first entry not symbol header", file);
            goto out;
        }

        esym = SARMAG + SAR_HDR + atolwhex(arhdr.size);
        off = SARMAG + SAR_HDR;

        /*
         * just bang the whole symbol file into memory
         */
        seek(f, off, 0);
        cnt = esym - off;
        start = malloc(cnt + 10);
        cnt = read(f, start, cnt);
        if(cnt <= 0){
            close(f);
            return;
        }
        stop = &start[cnt];
        memset(stop, 0, 10);

        work = true;

        while(work) {

            DBG("%5.2f library pass: %s\n", cputime(), file);

            work = false;
            for(e = start; e < stop; e = strchr(e+5, 0) + 1) {
                s = lookup(e+5, 0);
                if(s->type != SXREF)
                    continue;
                sprint(pname, "%s(%s)", file, s->name);

                DBG("%5.2f library: %s\n", cputime(), pname);

                l = e[1] & 0xff;
                l |= (e[2] & 0xff) << 8;
                l |= (e[3] & 0xff) << 16;
                l |= (e[4] & 0xff) << 24;
                seek(f, l, 0);
                /* need readn to read the dumps (at least) */
                l = readn(f, &arhdr, SAR_HDR);
                if(l != SAR_HDR)
                    goto bad;
                if(strncmp(arhdr.fmag, ARFMAG, sizeof(arhdr.fmag)))
                    goto bad;
                l = atolwhex(arhdr.size);
                ldobj(f, l, pname);
                if(s->type == SXREF) {
                    diag("%s: failed to load: %s", file, s->name);
                    errorexit();
                }
                work = true;
                xrefresolv = true;
            }
        }
        return;

    bad:
        diag("%s: bad or out of date archive", file);
    out:
        close(f);
    /*e: [[objfile()]] when file is a library */
}
/*e: function objfile */

/*s: function zaddr(arm) */
int
zaddr(byte *p, Adr *a, Sym *h[])
{
    int i, c;
    int l;
    Sym *s;
    Auto *u;

    a->type = p[0];
    a->reg = p[1];
    c = p[2];
    /*s: [[zaddr()]] sanity check symbol range */
    if(c < 0 || c > NSYM){
        print("sym out of range: %d\n", c);
        p[0] = ALAST+1;
        return 0;
    }
    /*e: [[zaddr()]] sanity check symbol range */
    a->sym = h[c];
    a->symkind = p[3];

    c = 4;

    /*s: [[zaddr()]] sanity check register range */
    if(a->reg < 0 || a->reg > NREG) {
        print("register out of range %d\n", a->reg);
        p[0] = ALAST+1;
        return 0;	/*  force real diagnostic */
    }
    /*e: [[zaddr()]] sanity check register range */

    switch(a->type) {
    /*s: [[zaddr()]] cases */
    case D_NONE:
    case D_REG:
    case D_FREG:
    case D_PSR:
    case D_FPCR:
        break;

    case D_REGREG:
        a->offset = p[4];
        c++;
        break;

    case D_BRANCH:
    case D_OREG:
    case D_CONST:
    case D_SHIFT:
        a->offset = p[4] | (p[5]<<8) | (p[6]<<16) | (p[7]<<24);
        c += 4;
        break;

    case D_SCONST:
        a->sval = malloc(NSNAME);
        memmove(a->sval, p+4, NSNAME);
        c += NSNAME;
        break;
    /*x: [[zaddr()]] cases */
    case D_FCONST:
        a->ieee = malloc(sizeof(Ieee));

        a->ieee->l = p[4] | (p[5]<<8) | (p[6]<<16) | (p[7]<<24);
        a->ieee->h = p[8] | (p[9]<<8) | (p[10]<<16) | (p[11]<<24);
        c += 8;
        break;
    /*e: [[zaddr()]] cases */
    default:
        print("unknown type %d\n", a->type);
        p[0] = ALAST+1;
        return 0;	/*  force real diagnostic */

    }

    s = a->sym;
    if(s == S)
        return c;
    i = a->symkind;
    if(i != D_AUTO && i != D_PARAM)
        return c;

    l = a->offset;
    for(u=curauto; u; u=u->link)
        if(u->asym == s)
         if(u->type == i) {
            if(u->aoffset > l)
                u->aoffset = l;
            return c;
        }

    u = malloc(sizeof(Auto));
    u->asym = s;
    u->aoffset = l;
    u->type = i;

    //add_list(u, curauto)
    u->link = curauto;
    curauto = u;

    return c;
}
/*e: function zaddr(arm) */

/*s: function addlib */
void
addlib(char *obj)
{
    char fn1[LIBNAMELEN], fn2[LIBNAMELEN], comp[LIBNAMELEN], *p, *name;
    int i, search;

    if(histfrogp <= 0)
        return;

    name = fn1;
    search = 0;
    if(histfrog[0]->name[1] == '/') {
        sprint(name, "");
        i = 1;
    } else if(histfrog[0]->name[1] == '.') {
        sprint(name, ".");
        i = 0;
    } else {
        sprint(name, "");
        i = 0;
        search = 1;
    }

    for(; i<histfrogp; i++) {
        snprint(comp, sizeof comp, histfrog[i]->name+1);
        for(;;) {
            p = strstr(comp, "$O");
            if(p == 0)
                break;
            memmove(p+1, p+2, strlen(p+2)+1);
            p[0] = thechar;
        }
        for(;;) {
            p = strstr(comp, "$M");
            if(p == 0)
                break;
            if(strlen(comp)+strlen(thestring)-2+1 >= sizeof comp) {
                diag("library component too long");
                return;
            }
            memmove(p+strlen(thestring), p+2, strlen(p+2)+1);
            memmove(p, thestring, strlen(thestring));
        }
        if(strlen(fn1) + strlen(comp) + 3 >= sizeof(fn1)) {
            diag("library component too long");
            return;
        }
        if(i > 0 || !search)
            strcat(fn1, "/");
        strcat(fn1, comp);
    }

    cleanname(name);

    if(search){
        p = findlib(name);
        if(p != nil){
            snprint(fn2, sizeof(fn2), "%s/%s", p, name);
            name = fn2;
        }
    }


    for(i=0; i<libraryp; i++)
        if(strcmp(name, library[i]) == 0)
            return;
    if(libraryp == nelem(library)){
        diag("too many autolibs; skipping %s", name);
        return;
    }

    p = malloc(strlen(name) + 1);
    strcpy(p, name);
    library[libraryp] = p;
    p = malloc(strlen(obj) + 1);
    strcpy(p, obj);
    libraryobj[libraryp] = p;
    libraryp++;
}
/*e: function addlib */

/*s: function addhist */
void
addhist(long line, int type)
{
    Auto *u;
    Sym *s;
    int i, j, k;

    u = malloc(sizeof(Auto));
    s = malloc(sizeof(Sym));
    s->name = malloc(2*(histfrogp+1) + 1);

    u->asym = s;
    u->type = type;
    u->aoffset = line;
    u->link = curhist;
    curhist = u;

    j = 1;
    for(i=0; i<histfrogp; i++) {
        k = histfrog[i]->value;
        s->name[j+0] = k>>8;
        s->name[j+1] = k;
        j += 2;
    }
}
/*e: function addhist */

/*s: function histtoauto */
void
histtoauto(void)
{
    Auto *l;

    while(l = curhist) {
        curhist = l->link;
        l->link = curauto;
        curauto = l;
    }
}
/*e: function histtoauto */

/*s: function collapsefrog */
void
collapsefrog(Sym *s)
{
    int i;

    /*
     * bad encoding of path components only allows
     * MAXHIST components. if there is an overflow,
     * first try to collapse xxx/..
     */
    for(i=1; i<histfrogp; i++)
        if(strcmp(histfrog[i]->name+1, "..") == 0) {
            memmove(histfrog+i-1, histfrog+i+1,
                (histfrogp-i-1)*sizeof(histfrog[0]));
            histfrogp--;
            goto out;
        }

    /*
     * next try to collapse .
     */
    for(i=0; i<histfrogp; i++)
        if(strcmp(histfrog[i]->name+1, ".") == 0) {
            memmove(histfrog+i, histfrog+i+1,
                (histfrogp-i-1)*sizeof(histfrog[0]));
            goto out;
        }

    /*
     * last chance, just truncate from front
     */
    memmove(histfrog+0, histfrog+1,
        (histfrogp-1)*sizeof(histfrog[0]));

out:
    histfrog[histfrogp-1] = s;
}
/*e: function collapsefrog */

/*s: function nopout */
void
nopout(Prog *p)
{
    p->as = ANOP;
    p->from.type = D_NONE;
    p->to.type = D_NONE;
}
/*e: function nopout */

/*s: function readsome */
byte*
readsome(int f, byte *buf, byte *good, byte *stop, int max)
{
    int n;

    n = stop - good;
    memmove(buf, good, stop - good);
    stop = buf + n;
    n = MAXIO - n;
    if(n > max)
        n = max;
    n = read(f, stop, n);
    if(n <= 0)
        return 0;
    return stop + n;
}
/*e: function readsome */

/*s: function ldobj(arm) */
void
ldobj(fdt f, long c, char *pn)
{
    /*s: [[ldobj()]] locals(arm) */
    long ipc;
    /*x: [[ldobj()]] locals(arm) */
    byte *bloc;
    byte *bsize;
    int r;
    /*x: [[ldobj()]] locals(arm) */
    // enum<opcode>
    int o;
    Prog *p;
    /*x: [[ldobj()]] locals(arm) */
    bool skip;
    /*x: [[ldobj()]] locals(arm) */
    Prog *t;
    int v;
    ulong sig;
    /*x: [[ldobj()]] locals(arm) */
    byte *stop;
    /*x: [[ldobj()]] locals(arm) */
    Sym *h[NSYM];
    Sym *s;
    /*x: [[ldobj()]] locals(arm) */
    // growing_array<filename>  (grown for every 16 elements)
    static char **filen;
    // index of next free entry in filen
    static int files = 0;
    /*x: [[ldobj()]] locals(arm) */
    char **nfilen;
    /*x: [[ldobj()]] locals(arm) */
    Sym *di;
    /*e: [[ldobj()]] locals(arm) */

    /*s: [[ldobj()]] remember set of object filenames */
    /*s: [[ldobj()]] grow filen if not enough space */
    if((files&15) == 0){
        nfilen = malloc((files+16)*sizeof(char*));
        memmove(nfilen, filen, files*sizeof(char*));
        free(filen);
        filen = nfilen;
    }
    /*e: [[ldobj()]] grow filen if not enough space */
    filen[files++] = strdup(pn);
    /*e: [[ldobj()]] remember set of object filenames */
    /*s: [[ldobj()]] bloc and bsize init */
    bsize = buf.xbuf;
    bloc = buf.xbuf;
    /*e: [[ldobj()]] bloc and bsize init */

    di = S;

// can come from AEND
newloop:
    ipc = pc;
    version++;
    skip = false;

    memset(h, 0, sizeof(h));
    histfrogp = 0;

loop:
    if(c <= 0)
        goto eof;

    /*s: [[ldobj()]] read if needed in loop:, adjust bloc and bsize */
    r = bsize - bloc;
    if(r < 100 && r < c) {		/* enough for largest prog */
        bsize = readsome(f, buf.xbuf, bloc, bsize, c);
        if(bsize == 0)
            goto eof;
        bloc = buf.xbuf;
        goto loop;
    }
    /*e: [[ldobj()]] read if needed in loop:, adjust bloc and bsize */

    o = bloc[0];		/* as */
    /*s: [[ldobj()]] sanity check opcode in range(arm) */
    if(o <= AXXX || o >= ALAST) {
        diag("%s: line %ld: opcode out of range %d", pn, pc-ipc, o);
        print("	probably not a .5 file\n");
        errorexit();
    }
    /*e: [[ldobj()]] sanity check opcode in range(arm) */
    /*s: [[ldobj()]] if ANAME or ASIGNAME(arm) */
    if(o == ANAME || o == ASIGNAME) {
        sig = 0;
        /*s: [[ldobj()]] if SIGNAME adjust sig */
        if(o == ASIGNAME){
            sig = bloc[1] | (bloc[2]<<8) | (bloc[3]<<16) | (bloc[4]<<24);
            bloc += 4;
            c -= 4;
        }
        /*e: [[ldobj()]] if SIGNAME adjust sig */

        stop = memchr(&bloc[3], '\0', bsize-&bloc[3]);
        /*s: [[ldobj()]] if stop is nil refill buffer and retry */
        if(stop == nil){
            bsize = readsome(f, buf.xbuf, bloc, bsize, c);
            if(bsize == 0)
                goto eof;
            bloc = buf.xbuf;
            stop = memchr(&bloc[3], '\0', bsize-&bloc[3]);
            if(stop == nil){
                fprint(2, "%s: name too long\n", pn);
                errorexit();
            }
        }
        /*e: [[ldobj()]] if stop is nil refill buffer and retry */

        v = bloc[1];	/* type */
        o = bloc[2];	/* sym */
        bloc += 3;
        c -= 3;

        r = 0;
        if(v == D_STATIC)
            r = version;

        s = lookup((char*)bloc, r);
        c -= &stop[1] - bloc;
        bloc = stop + 1;

        /*s: [[ldobj()]] if sig not zero */
        if(sig != 0){
            if(s->sig != 0 && s->sig != sig)
                diag("incompatible type signatures %lux(%s) and %lux(%s) for %s", s->sig, filen[s->file], sig, pn, s->name);
            s->sig = sig;
            s->file = files-1;
        }
        /*e: [[ldobj()]] if sig not zero */

        if(debug['W'])
            print("	ANAME	%s\n", s->name);

        h[o] = s;

        if((v == D_EXTERN || v == D_STATIC) && s->type == SNONE)
            s->type = SXREF;

        /*s: [[ldobj()]] when ANAME opcode, if D_FILE */
        if(v == D_FILE) {
            if(s->type != SFILE) {
                histgen++;
                s->type = SFILE;
                s->value = histgen;
            }
            if(histfrogp < MAXHIST) {
                histfrog[histfrogp] = s;
                histfrogp++;
            } else
                collapsefrog(s);
        }
        /*e: [[ldobj()]] when ANAME opcode, if D_FILE */
        goto loop;
    }
    /*e: [[ldobj()]] if ANAME or ASIGNAME(arm) */
    p = malloc(sizeof(Prog));
    p->as = o;
    // reading the object binary file, opposite of outcode() in Assembler.nw
    /*s: [[ldobj()]] read one instruction in p */
    p->scond = bloc[1];
    p->reg   = bloc[2];
    p->line  = bloc[3] | (bloc[4]<<8) | (bloc[5]<<16) | (bloc[6]<<24);
    r = 7;
    r += zaddr(bloc+7, &p->from, h);
    r += zaddr(bloc+r, &p->to, h);

    bloc += r;
    c -= r;

    p->link = P;
    p->cond = P;
    /*e: [[ldobj()]] read one instruction in p */
    /*s: [[ldobj()]] sanity check p */
    if(p->reg > NREG)
        diag("register out of range %d", p->reg);
    /*e: [[ldobj()]] sanity check p */

    if(debug['W'])
        print("%P\n", p);

    switch(o) {
    /*s: [[ldobj()]] switch opcode cases(arm) */
    case ATEXT:
        if(curtext != P) {
            histtoauto();
            curtext->to.autom = curauto;
            curauto = nil;
        }
        skip = false; // needed?

        curtext = p;

        autosize = (p->to.offset+3L) & ~3L;
        p->to.offset = autosize;
        autosize += 4;

        s = p->from.sym;

        if(s == S) {
            diag("TEXT must have a name\n%P", p);
            errorexit();
        }

        if(s->type != SNONE && s->type != SXREF) {
            if(p->reg & DUPOK) {
                skip = true;
                goto casedef;
            }
            diag("redefinition: %s\n%P", s->name, p);
        }

        s->type = STEXT;
        s->value = pc;

        // like in default case
        //add_list(firstp, lastp, p)
        lastp->link = p;
        lastp = p;
        p->pc = pc;
        pc++;

        //add_list(textp, etextp, p)
        if(textp == P) {
            textp = p;
            etextp = p;
        } else {
            etextp->cond = p;
            etextp = p;
        }
        break;
    /*x: [[ldobj()]] switch opcode cases(arm) */
    case ADATA:
        if(p->from.sym == S) {
            diag("DATA without a sym\n%P", p);
            break;
        }
        //add_list(datap, edatap, p)
        p->link = datap;
        datap = p;

        break;
    /*x: [[ldobj()]] switch opcode cases(arm) */
    case AGLOBL:
        s = p->from.sym;
        if(s == S) {
            diag("GLOBL must have a name\n%P", p);
            errorexit();
        }

        if(s->type == SNONE || s->type == SXREF) {
            s->type = SBSS;
            s->value = 0;
        }
        if(s->type != SBSS) {
            diag("redefinition: %s\n%P", s->name, p);
            s->type = SBSS;
            s->value = 0;
        }
        if(p->to.offset > s->value)
            s->value = p->to.offset;
        break;
    /*x: [[ldobj()]] switch opcode cases(arm) */
    case AEND:

        histtoauto();
        if(curtext != P)
            curtext->to.autom = curauto;
        curauto = nil;

        curtext = P;
        if(c)
            goto newloop;
        return;
    /*x: [[ldobj()]] switch opcode cases(arm) */
    case AHISTORY:
        if(p->to.offset == -1) {
            addlib(pn);
            histfrogp = 0;
            goto loop;
        }
        addhist(p->line, D_FILE);		/* 'z' */
        if(p->to.offset)
            addhist(p->to.offset, D_FILE1);	/* 'Z' */
        histfrogp = 0;
        goto loop;
    /*x: [[ldobj()]] switch opcode cases(arm) */
    case ASUB:
        if(p->from.type == D_CONST)
         if(p->from.symkind == D_NONE)
          if(p->from.offset < 0) {
            p->from.offset = -p->from.offset;
            p->as = AADD;
        }
        goto casedef;
    /*x: [[ldobj()]] switch opcode cases(arm) */
    case AADD:
        if(p->from.type == D_CONST)
         if(p->from.symkind == D_NONE)
          if(p->from.offset < 0) {
            p->from.offset = -p->from.offset;
            p->as = ASUB;
        }
        goto casedef;
    /*x: [[ldobj()]] switch opcode cases(arm) */
    case ADYNT:
        if(p->to.sym == S) {
            diag("DYNT without a sym\n%P", p);
            break;
        }
        di = p->to.sym;
        p->reg = 4;
        if(di->type == SXREF) {
            if(debug['z'])
                Bprint(&bso, "%P set to %d\n", p, dtype);
            di->type = SCONST;
            di->value = dtype;
            dtype += 4;
        }
        if(p->from.sym == S)
            break;

        p->from.offset = di->value;
        p->from.sym->type = SDATA;
        if(curtext == P) {
            diag("DYNT not in text: %P", p);
            break;
        }
        p->to.sym = curtext->from.sym;
        p->to.type = D_CONST;
        p->link = datap;
        datap = p;
        break;
    /*x: [[ldobj()]] switch opcode cases(arm) */
    case AINIT:
        if(p->from.sym == S) {
            diag("INIT without a sym\n%P", p);
            break;
        }
        if(di == S) {
            diag("INIT without previous DYNT\n%P", p);
            break;
        }
        p->from.offset = di->value;
        p->from.sym->type = SDATA;
        p->link = datap;
        datap = p;
        break;

    /*x: [[ldobj()]] switch opcode cases(arm) */
    case AGOK:
        diag("unknown opcode\n%P", p);
        p->pc = pc;
        pc++;
        break;
    /*x: [[ldobj()]] switch opcode cases(arm) */
    case AMOVDF:
        if(!vfp || p->from.type != D_FCONST)
            goto casedef;
        p->as = AMOVF;
        /* fall through */
    case AMOVF:
        if(skip)
            goto casedef;

        if(p->from.type == D_FCONST && chipfloat(p->from.ieee) < 0) {
            /* size sb 9 max */
            sprint(literal, "$%lux", ieeedtof(p->from.ieee));
            s = lookup(literal, 0);
            if(s->type == 0) {
                s->type = SBSS;
                s->value = 4;
                t = prg();
                t->as = ADATA;
                t->line = p->line;
                t->from.type = D_OREG;
                t->from.sym = s;
                t->from.symkind = D_EXTERN;
                t->reg = 4;
                t->to = p->from;
                t->link = datap;
                datap = t;
            }
            p->from.type = D_OREG;
            p->from.sym = s;
            p->from.symkind = D_EXTERN;
            p->from.offset = 0;
        }
        goto casedef;

    case AMOVD:
        if(skip)
            goto casedef;

        if(p->from.type == D_FCONST && chipfloat(p->from.ieee) < 0) {
            /* size sb 18 max */
            sprint(literal, "$%lux.%lux",
                p->from.ieee->l, p->from.ieee->h);
            s = lookup(literal, 0);
            if(s->type == 0) {
                s->type = SBSS;
                s->value = 8;
                t = prg();
                t->as = ADATA;
                t->line = p->line;
                t->from.type = D_OREG;
                t->from.sym = s;
                t->from.symkind = D_EXTERN;
                t->reg = 8;
                t->to = p->from;
                t->link = datap;
                datap = t;
            }
            p->from.type = D_OREG;
            p->from.sym = s;
            p->from.symkind = D_EXTERN;
            p->from.offset = 0;
        }
        goto casedef;
    /*e: [[ldobj()]] switch opcode cases(arm) */
    default:
    casedef:
        if(skip)
            nopout(p);

        // putting each object after each other, local offset become global
        if(p->to.type == D_BRANCH)
            p->to.offset += ipc;

        //add_list(firstp, lastp, p)
        lastp->link = p;
        lastp = p;

        p->pc = pc;
        pc++;
        break;
    }
    goto loop;

eof:
    diag("truncated object file: %s", pn);
}
/*e: function ldobj(arm) */

/*s: function doprof1(arm) */
void
doprof1(void)
{
    Sym *s;
    long n;
    Prog *p, *q;

    DBG("%5.2f profile 1\n", cputime());
    s = lookup("__mcount", 0);
    n = 1;
    for(p = firstp->link; p != P; p = p->link) {
        if(p->as == ATEXT) {
            q = prg();
            q->line = p->line;
            q->link = datap;
            datap = q;
            q->as = ADATA;
            q->from.type = D_OREG;
            q->from.symkind = D_EXTERN;
            q->from.offset = n*4;
            q->from.sym = s;
            q->reg = 4;
            q->to = p->from;
            q->to.type = D_CONST;

            q = prg();
            q->line = p->line;
            q->pc = p->pc;
            q->link = p->link;
            p->link = q;
            p = q;
            p->as = AMOVW;
            p->from.type = D_OREG;
            p->from.symkind = D_EXTERN;
            p->from.sym = s;
            p->from.offset = n*4 + 4;
            p->to.type = D_REG;
            p->to.reg = REGTMP;

            q = prg();
            q->line = p->line;
            q->pc = p->pc;
            q->link = p->link;
            p->link = q;
            p = q;
            p->as = AADD;
            p->from.type = D_CONST;
            p->from.offset = 1;
            p->to.type = D_REG;
            p->to.reg = REGTMP;

            q = prg();
            q->line = p->line;
            q->pc = p->pc;
            q->link = p->link;
            p->link = q;
            p = q;
            p->as = AMOVW;
            p->from.type = D_REG;
            p->from.reg = REGTMP;
            p->to.type = D_OREG;
            p->to.symkind = D_EXTERN;
            p->to.sym = s;
            p->to.offset = n*4 + 4;

            n += 2;
            continue;
        }
    }
    q = prg();
    q->line = 0;
    q->link = datap;
    datap = q;

    q->as = ADATA;
    q->from.type = D_OREG;
    q->from.symkind = D_EXTERN;
    q->from.sym = s;
    q->reg = 4;
    q->to.type = D_CONST;
    q->to.offset = n;

    s->type = SBSS;
    s->value = n*4;
}
/*e: function doprof1(arm) */

/*s: global brcond(arm) */
static int brcond[] = {ABEQ, ABNE, ABCS, ABCC, ABMI, ABPL, ABVS, ABVC, ABHI, ABLS, ABGE, ABLT, ABGT, ABLE};
/*e: global brcond(arm) */

/*s: function doprof2(arm) */
void
doprof2(void)
{
    Sym *s2, *s4;
    Prog *p, *q, *q2;
    Prog *ps2, *ps4;

    DBG("%5.2f profile 2\n", cputime());

    /*s: [[doprof2()]] if embedded tracing */
    if(debug['e']){
        s2 = lookup("_tracein", 0);
        s4 = lookup("_traceout", 0);
    }
    /*e: [[doprof2()]] if embedded tracing */
    else{
        s2 = lookup("_profin", 0);
        s4 = lookup("_profout", 0);
    }
    if(s2->type != STEXT || s4->type != STEXT) {
       /*s: [[doprof2()]] if embedded tracing diag() */
       if(debug['e'])
           diag("_tracein/_traceout not defined %d %d", s2->type, s4->type);
       /*e: [[doprof2()]] if embedded tracing diag() */
        else
            diag("_profin/_profout not defined");
        return;
    }

    // finding ps2, ps4 = instruction (Prog) of s2 and s4
    ps2 = P;
    ps4 = P;
    for(p = firstp; p != P; p = p->link) {
        if(p->as == ATEXT) {
            if(p->from.sym == s2) {
                ps2 = p;
                p->reg = 1;
            }
            if(p->from.sym == s4) {
                ps4 = p;
                p->reg = 1;
            }
        }
    }
    for(p = firstp; p != P; p = p->link) {
        if(p->as == ATEXT) {

            /*s: [[doprof2()]] if NOPROF p(arm) */
            if(p->reg & NOPROF) {
                for(;;) {
                    q = p->link;
                    if(q == P)
                        break;
                    if(q->as == ATEXT)
                        break;
                    p = q;
                }
                continue;
            }
            /*e: [[doprof2()]] if NOPROF p(arm) */

            /*
             * BL	profin
             */
            q = prg();
            q->line = p->line;
            q->pc = p->pc;
            q->link = p->link;

            /*s: [[doprof2()]] if embedded tracing ATEXT instrumentation(arm) */
            if(debug['e']){		/* embedded tracing */
                q2 = prg();
                p->link = q2;
                q2->link = q;

                q2->line = p->line;
                q2->pc = p->pc;

                q2->as = AB;
                q2->to.type = D_BRANCH;
                q2->to.sym = p->to.sym;
                q2->cond = q->link;
            }
            /*e: [[doprof2()]] if embedded tracing ATEXT instrumentation(arm) */
            else
                p->link = q;
            p = q;
            p->as = ABL;
            p->to.type = D_BRANCH;
            p->cond = ps2;
            p->to.sym = s2;

            continue;
        }
        if(p->as == ARET) {
            /*s: [[doprof2()]] if embedded tracing ARET instrumentation */
            /*
             * RET (default)
             */
            if(debug['e']){		/* embedded tracing */
                q = prg();
                q->line = p->line;
                q->pc = p->pc;
                q->link = p->link;
                p->link = q;
                p = q;
            }
            /*e: [[doprof2()]] if embedded tracing ARET instrumentation */
            /*
             * RET
             */
            q = prg();
            q->as = ARET;
            q->from = p->from;
            q->to = p->to;
            q->cond = p->cond;
            q->link = p->link;
            q->reg = p->reg;
            p->link = q;

            if(p->scond != COND_ALWAYS) {
                q = prg();
                q->as = ABL;
                q->from = zprg.from;
                q->to = zprg.to;
                q->to.type = D_BRANCH;
                q->cond = ps4;
                q->to.sym = s4;
                q->link = p->link;
                p->link = q;

                p->as = brcond[p->scond^1];	/* complement */
                p->scond = COND_ALWAYS;
                p->from = zprg.from;
                p->to = zprg.to;
                p->to.type = D_BRANCH;
                p->cond = q->link->link;	/* successor of RET */
                p->to.offset = q->link->link->pc;

                p = q->link->link;
            } else {

                /*
                 * BL	profout
                 */
                p->as = ABL;
                p->from = zprg.from;
                p->to = zprg.to;
                p->to.type = D_BRANCH;
                p->cond = ps4;
                p->to.sym = s4;
                p->scond = COND_ALWAYS;

                p = q;
            }
            continue;
        }
    }
}
/*e: function doprof2(arm) */

/*s: function nuxiinit(arm) */
void
nuxiinit(void)
{

    int i, c;

    for(i=0; i<4; i++) {
        c = find1(0x04030201L, i+1);
        if(i < 2)
            inuxi2[i] = c;
        if(i < 1)
            inuxi1[i] = c;
        inuxi4[i] = c;
        fnuxi4[i] = c;
        if(debug['d'] == 0){
            fnuxi8[i] = c;
            fnuxi8[i+4] = c+4;
        }
        else{
            fnuxi8[i] = c+4;		/* ms word first, then ls, even in little endian mode */
            fnuxi8[i+4] = c;
        }
    }
    if(debug['v']) {
        Bprint(&bso, "inuxi = ");
        for(i=0; i<1; i++)
            Bprint(&bso, "%d", inuxi1[i]);
        Bprint(&bso, " ");
        for(i=0; i<2; i++)
            Bprint(&bso, "%d", inuxi2[i]);
        Bprint(&bso, " ");
        for(i=0; i<4; i++)
            Bprint(&bso, "%d", inuxi4[i]);
        Bprint(&bso, "\nfnuxi = ");
        for(i=0; i<4; i++)
            Bprint(&bso, "%d", fnuxi4[i]);
        Bprint(&bso, " ");
        for(i=0; i<8; i++)
            Bprint(&bso, "%d", fnuxi8[i]);
        Bprint(&bso, "\n");
    }
    Bflush(&bso);
}
/*e: function nuxiinit(arm) */

/*s: function find1 */
int
find1(long l, int c)
{
    char *p;
    int i;

    p = (char*)&l;
    for(i=0; i<4; i++)
        if(*p++ == c)
            return i;
    return 0;
}
/*e: function find1 */

/*s: function ieeedtof */
long
ieeedtof(Ieee *e)
{
    int exp;
    long v;

    if(e->h == 0)
        return 0;
    exp = (e->h>>20) & ((1L<<11)-1L);
    exp -= (1L<<10) - 2L;
    v = (e->h & 0xfffffL) << 3;
    v |= (e->l >> 29) & 0x7L;
    if((e->l >> 28) & 1) {
        v++;
        if(v & 0x800000L) {
            v = (v & 0x7fffffL) >> 1;
            exp++;
        }
    }
    if(exp <= -126 || exp >= 130)
        diag("double fp to single fp overflow");
    v |= ((exp + 126) & 0xffL) << 23;
    v |= e->h & 0x80000000L;
    return v;
}
/*e: function ieeedtof */

/*s: function ieeedtod */
double
ieeedtod(Ieee *ieeep)
{
    Ieee e;
    double fr;
    int exp;

    if(ieeep->h & (1L<<31)) {
        e.h = ieeep->h & ~(1L<<31);
        e.l = ieeep->l;
        return -ieeedtod(&e);
    }
    if(ieeep->l == 0 && ieeep->h == 0)
        return 0;
    fr = ieeep->l & ((1L<<16)-1L);
    fr /= 1L<<16;
    fr += (ieeep->l>>16) & ((1L<<16)-1L);
    fr /= 1L<<16;
    fr += (ieeep->h & (1L<<20)-1L) | (1L<<20);
    fr /= 1L<<21;
    exp = (ieeep->h>>20) & ((1L<<11)-1L);
    exp -= (1L<<10) - 2L;
    return ldexp(fr, exp);
}
/*e: function ieeedtod */

/*s: function undefsym */
void
undefsym(Sym *s)
{
    int n;

    n = imports;
    if(s->value != 0)
        diag("value != 0 on SXREF");
    if(n >= 1<<Rindex)
        diag("import index %d out of range", n);
    s->value = n<<Roffset;
    s->type = SUNDEF;
    imports++;
}
/*e: function undefsym */

/*s: function zerosig */
void
zerosig(char *sp)
{
    Sym *s;

    s = lookup(sp, 0);
    s->sig = 0;
}
/*e: function zerosig */

/*s: function readundefs */
void
readundefs(char *f, int t)
{
    int i, n;
    Sym *s;
    Biobuf *b;
    char *l, buf[256], *fields[64];

    if(f == nil)
        return;
    b = Bopen(f, OREAD);
    if(b == nil){
        diag("could not open %s: %r", f);
        errorexit();
    }
    while((l = Brdline(b, '\n')) != nil){
        n = Blinelen(b);
        if(n >= sizeof(buf)){
            diag("%s: line too long", f);
            errorexit();
        }
        memmove(buf, l, n);
        buf[n-1] = '\0';
        n = getfields(buf, fields, nelem(fields), 1, " \t\r\n");
        if(n == nelem(fields)){
            diag("%s: bad format", f);
            errorexit();
        }
        for(i = 0; i < n; i++){
            s = lookup(fields[i], 0);
            s->type = SXREF;
            s->subtype = t;
            if(t == SIMPORT)
                nimports++;
            else
                nexports++;
        }
    }
    Bterm(b);
}
/*e: function readundefs */
/*e: linkers/5l/obj.c */
