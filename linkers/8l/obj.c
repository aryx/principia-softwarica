/*s: linkers/8l/obj.c */
#include	"l.h"
#include	<ar.h>

#ifndef	DEFAULT
/*s: constant DEFAULT */
#define	DEFAULT	'9'
/*e: constant DEFAULT */
#endif

/*s: global curauto */
Auto*	curauto;
/*e: global curauto */

/*s: global curhist */
Auto*	curhist;
/*e: global curhist */
/*s: global etextp */
// ref<Prog>, end of textp list
Prog*	etextp = P;
/*e: global etextp */

/*s: global histfrog */
Sym*	histfrog[MAXHIST];
/*e: global histfrog */
/*s: global histfrogp */
int	histfrogp;
/*e: global histfrogp */
/*s: global histgen */
int	histgen = 0;
/*e: global histgen */

/*s: global library */
// array<option<filename>>
char*	library[50];
/*e: global library */
/*s: global libraryobj */
char*	libraryobj[50];
/*e: global libraryobj */
/*s: global libraryp */
// index of first free entry in library array
int	libraryp;
/*e: global libraryp */

/*s: global xrefresolv */
bool	xrefresolv;
/*e: global xrefresolv */

/*s: global version */
int	version = 0;
/*e: global version */
/*s: global literal(x86) */
char	literal[32];
/*e: global literal(x86) */
/*s: global doexp */
// do export table, -x
bool	doexp;
/*e: global doexp */

void	addlibpath(char*);
char*	findlib(char*);
void	loadlib(void);
void	objfile(char*);

int	zaddr(uchar*, Adr*, Sym*[]);
long	vaddr(Adr*);

void	addhist(long, int);
void	histtoauto(void);
void	ldobj(int, long, char*);

void	doprof1(void);
void	doprof2(void);
void	nuxiinit(void);

int	find1(long, int);
//int	find2(long, int);

double	ieeedtod(Ieee*);

void	zerosig(char*);
void	readundefs(char*, int);
Prog*	brchain(Prog*);



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
char*	thestring;
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

/*s: function main (linkers/8l/obj.c) */
void
main(int argc, char *argv[])
{
    /*s: [[main()]] locals(x86) */
        int i, c;
        char name[LIBNAMELEN];
        char *a;
    /*x: [[main()]] locals(x86) */
    bool load_libs;
    /*x: [[main()]] locals(x86) */
    char *root;
    /*e: [[main()]] locals(x86) */

    thechar = '8';
    thestring = "386";   
    outfile = "8.out";

    /*s: [[main()]] debug initialization(x86) */
    Binit(&bso, 1, OWRITE);
    listinit(); // fmtinstall()
    memset(debug, false, sizeof(debug));
    /*e: [[main()]] debug initialization(x86) */

    ARGBEGIN {
    /*s: [[main()]] command line processing(x86) */
        case 'o': /* output to (next arg) */
            outfile = ARGF();
            break;
    /*x: [[main()]] command line processing(x86) */
    case 'H':
        a = ARGF();
        if(a)
            HEADTYPE = atolwhex(a);
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
    /*x: [[main()]] command line processing(x86) */
    case 'L':
        addlibpath(EARGF(usage()));
        break;
    /*x: [[main()]] command line processing(x86) */
    case 'x':	/* produce export table */
        doexp = true;
        if(argv[1] != nil && argv[1][0] != '-' && !isobjfile(argv[1])){
            a = ARGF();
            if(strcmp(a, "*") == 0)
                allexport = true;
            else
                readundefs(a, SEXPORT);
        }
        break;
    /*x: [[main()]] command line processing(x86) */
    case 'u':	/* produce dynamically loadable module */
        dlm = true;
        // do not load standard libraries
        debug['l'] = true;

        if(argv[1] != nil && argv[1][0] != '-' && !isobjfile(argv[1]))
            readundefs(ARGF(), SIMPORT);
        break;
    /*x: [[main()]] command line processing(x86) */
    case 'P':
        a = ARGF();
        if(a)
            INITTEXTP = atolwhex(a);
        break;
    /*x: [[main()]] command line processing(x86) */
    default:
        c = ARGC();
        if(c >= 0 && c < sizeof(debug))
            debug[c] = true;
        break;
    /*e: [[main()]] command line processing(x86) */
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

    /*s: [[main()]] adjust HEADTYPE if debug flags(x86) */
    if(!debug['9'] && !debug['U'] && !debug['B'])
        debug[DEFAULT] = true;

    if(HEADTYPE == -1) {
        if(debug['U'])
            HEADTYPE = 1;
        if(debug['B'])
            HEADTYPE = 2;
        if(debug['9'])
            HEADTYPE = 2;
    }
    /*e: [[main()]] adjust HEADTYPE if debug flags(x86) */
    switch(HEADTYPE) {
    /*s: [[main()]] switch HEADTYPE cases(x86) */
    case H_GARBAGE:	/* this is garbage */
        HEADR = 20L+56L;
        if(INITTEXT == -1)
            INITTEXT = 0x40004CL;
        if(INITDAT == -1)
            INITDAT = 0x10000000L;
        if(INITRND == -1)
            INITRND = 0;
        break;
    case H_COFF:	/* is unix coff */
        HEADR = 0xd0L;
        if(INITTEXT == -1)
            INITTEXT = 0xd0;
        if(INITDAT == -1)
            INITDAT = 0x400000;
        if(INITRND == -1)
            INITRND = 0;
        break;
    case H_COM:	/* MS-DOS .COM */
        HEADR = 0;
        if(INITTEXT == -1)
            INITTEXT = 0x0100;
        if(INITDAT == -1)
            INITDAT = 0;
        if(INITRND == -1)
            INITRND = 4;
        break;
    case H_EXE:	/* fake MS-DOS .EXE */
        HEADR = 0x200;
        if(INITTEXT == -1)
            INITTEXT = 0x0100;
        if(INITDAT == -1)
            INITDAT = 0;
        if(INITRND == -1)
            INITRND = 4;
        HEADR += (INITTEXT & 0xFFFF);
        DBG("HEADR = 0x%ld\n", HEADR);
        break;
    case H_ELF:	/* elf executable */
        HEADR = rnd(Ehdr32sz+3*Phdr32sz, 16);
        if(INITTEXT == -1)
            INITTEXT = 0x80100020L;
        if(INITDAT == -1)
            INITDAT = 0;
        if(INITRND == -1)
            INITRND = 4096;
        break;
    /*x: [[main()]] switch HEADTYPE cases(x86) */
    case H_PLAN9:	/* plan 9 */
        HEADR = 32L;
        if(INITTEXT == -1)
            INITTEXT = 4096+32;
        if(INITDAT == -1)
            INITDAT = 0;
        if(INITRND == -1)
            INITRND = 4096;
        break;
    /*e: [[main()]] switch HEADTYPE cases(x86) */
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

    DBG("HEADER = -H0x%ld -T0x%lux -D0x%lux -R0x%lux\n",
            HEADTYPE, INITTEXT, INITDAT, INITRND);

    /*s: [[main()]] sanity check optab(x86) */
    for(i=1; optab[i].as; i++)
        if(i != optab[i].as) {
            diag("phase error in optab: %d", i);
            errorexit();
        }
    /*e: [[main()]] sanity check optab(x86) */
    /*s: [[main()]] initialize globals(x86) */
    /*s: [[main()]] set ycover(x86) */
    for(i=0; i<Ymax; i++)
        ycover[i*Ymax + i] = 1;

    ycover[Yi0*Ymax + Yi8] = 1;
    ycover[Yi1*Ymax + Yi8] = 1;

    ycover[Yi0*Ymax + Yi32] = 1;
    ycover[Yi1*Ymax + Yi32] = 1;
    ycover[Yi8*Ymax + Yi32] = 1;

    ycover[Yal*Ymax + Yrb] = 1;
    ycover[Ycl*Ymax + Yrb] = 1;
    ycover[Yax*Ymax + Yrb] = 1;
    ycover[Ycx*Ymax + Yrb] = 1;
    ycover[Yrx*Ymax + Yrb] = 1;

    ycover[Yax*Ymax + Yrx] = 1;
    ycover[Ycx*Ymax + Yrx] = 1;

    ycover[Yax*Ymax + Yrl] = 1;
    ycover[Ycx*Ymax + Yrl] = 1;
    ycover[Yrx*Ymax + Yrl] = 1;

    ycover[Yf0*Ymax + Yrf] = 1;

    ycover[Yal*Ymax + Ymb] = 1;
    ycover[Ycl*Ymax + Ymb] = 1;
    ycover[Yax*Ymax + Ymb] = 1;
    ycover[Ycx*Ymax + Ymb] = 1;
    ycover[Yrx*Ymax + Ymb] = 1;
    ycover[Yrb*Ymax + Ymb] = 1;
    ycover[Ym*Ymax + Ymb] = 1;

    ycover[Yax*Ymax + Yml] = 1;
    ycover[Ycx*Ymax + Yml] = 1;
    ycover[Yrx*Ymax + Yml] = 1;
    ycover[Yrl*Ymax + Yml] = 1;
    ycover[Ym*Ymax + Yml] = 1;
    /*e: [[main()]] set ycover(x86) */
    /*s: [[main()]] set reg(x86) */
    for(i=0; i<D_NONE; i++) {
        reg[i] = -1;
        if(i >= D_AL && i <= D_BH)
            reg[i] = (i-D_AL) & 7;
        if(i >= D_AX && i <= D_DI)
            reg[i] = (i-D_AX) & 7;
        if(i >= D_F0 && i <= D_F0+7)
            reg[i] = (i-D_F0) & 7;
    }
    /*e: [[main()]] set reg(x86) */
    /*s: [[main()]] set zprg(x86) */
    zprg.link = P;
    zprg.pcond = P;
    zprg.back = 2;
    zprg.as = AGOK;
    zprg.from.type = D_NONE;
    zprg.from.index = D_NONE;
    zprg.from.scale = 1;
    zprg.to = zprg.from;
    /*e: [[main()]] set zprg(x86) */
    dtype = 4;

    cbp = buf.obuf;
    cbc = sizeof(buf.obuf);
    /*x: [[main()]] initialize globals(x86) */
    load_libs = !debug['l'];
    /*e: [[main()]] initialize globals(x86) */

    nuxiinit();

    cout = create(outfile, 1, 0775);
    if(cout < 0) {
        diag("cannot create %s: %r", outfile);
        errorexit();
    }

    // ------ main functions  ------
    /*s: [[main()]] cout is ready, LET'S GO(x86) */
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

    while(*argv)
        objfile(*argv++);

    if(load_libs)
        loadlib();

    firstp = firstp->link;
    if(firstp == P)
        errorexit();

    /*s: [[main()]] if export table or dynamic module(x86) */
    if(doexp || dlm){
        EXPTAB = "_exporttab";
        zerosig(EXPTAB);
        zerosig("etext");
        zerosig("edata");
        zerosig("end");

       /*s: [[main()]] if dynamic module(x86) */
       if(dlm){
           import();
           HEADTYPE = H_PLAN9;
           INITTEXT = INITDAT = 0;
           INITRND = 8;
           INITENTRY = EXPTAB;
       }
       /*e: [[main()]] if dynamic module(x86) */

        export();
    }
    /*e: [[main()]] if export table or dynamic module(x86) */

    patch();
    follow();
    dodata();
    dostkoff();
    /*s: [[main()]] call doprofxxx() if profiling */
    if(debug['p'])
        if(debug['1'])
            doprof1();
        else
            doprof2();
    /*e: [[main()]] call doprofxxx() if profiling */
    span();
    doinit();

    // write to cout, finally
    asmb();

    // sanity check
    undef();
    /*e: [[main()]] cout is ready, LET'S GO(x86) */

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
/*e: function main (linkers/8l/obj.c) */

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
    /*s: [[loadlib()]] reset xrefresolv */
    xrefresolv = false;
    /*e: [[loadlib()]] reset xrefresolv */
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
    struct ar_hdr arhdr;
    long off, esym, cnt;
    bool work;
    Sym *s;
    char pname[LIBNAMELEN];
    char name[LIBNAMELEN];
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

/*s: function zaddr(x86) */
int
zaddr(byte *p, Adr *a, Sym *h[])
{
    int c, t, i;
    int l;
    Sym *s;
    Auto *u;

    t = p[0];

    c = 1;
    if(t & T_INDEX) {
        a->index = p[c];
        a->scale = p[c+1];
        c += 2;
    } else {
        a->index = D_NONE;
        a->scale = 0;
    }
    a->offset = 0;
    if(t & T_OFFSET) {
        a->offset = p[c] | (p[c+1]<<8) | (p[c+2]<<16) | (p[c+3]<<24);
        c += 4;
        // >> >> >>
    }
    a->sym = S;
    if(t & T_SYM) {
        a->sym = h[p[c]];
        c++;
    }
    a->type = D_NONE;
    if(t & T_FCONST) {
        a->ieee.l = p[c] | (p[c+1]<<8) | (p[c+2]<<16) | (p[c+3]<<24);
        a->ieee.h = p[c+4] | (p[c+5]<<8) | (p[c+6]<<16) | (p[c+7]<<24);
        // >> >> >> >> >> >>
        c += 8;
        a->type = D_FCONST;
    } else
    if(t & T_SCONST) {
        for(i=0; i<NSNAME; i++)
            a->scon[i] = p[c+i];
        c += NSNAME;
        a->type = D_SCONST;
    }
    if(t & T_TYPE) {
        a->type = p[c];
        c++;
    }
    s = a->sym;
    if(s == S)
        return c;

    t = a->type;
    if(t != D_AUTO && t != D_PARAM)
        return c;
    l = a->offset;
    for(u=curauto; u; u=u->link) {
        if(u->asym == s)
        if(u->type == t) {
            if(u->aoffset > l)
                u->aoffset = l;
            return c;
        }
    }

    // factorize!
    while(nhunk < sizeof(Auto))
        gethunk();
    u = (Auto*)hunk;
    nhunk -= sizeof(Auto);
    hunk += sizeof(Auto);

    u->link = curauto;
    curauto = u;
    u->asym = s;
    u->aoffset = l;
    u->type = t;
    return c;
}
/*e: function zaddr(x86) */

/*s: function addlib */
void
addlib(char *obj)
{
    char fn1[LIBNAMELEN], fn2[LIBNAMELEN], comp[LIBNAMELEN];
    char *p, *name;
    int i;
    bool search;

    if(histfrogp <= 0)
        return;

    name = fn1;
    search = false;
    if(histfrog[0]->name[1] == '/') {
        sprint(name, "");
        i = 1;
    } else if(histfrog[0]->name[1] == '.') {
        sprint(name, ".");
        i = 0;
    } else {
        sprint(name, "");
        i = 0;
        search = true;
    }

    for(; i<histfrogp; i++) {
        snprint(comp, sizeof comp, histfrog[i]->name+1);

        // s/$0/<thechar>/
        for(;;) {
            p = strstr(comp, "$O");
            if(p == nil)
                break;
            memmove(p+1, p+2, strlen(p+2)+1);
            p[0] = thechar;
        }
        // s/$M/<thestring>/
        for(;;) {
            p = strstr(comp, "$M");
            if(p == nil)
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

    s = malloc(sizeof(Sym));
    s->name = malloc(2*(histfrogp+1) + 1);

    u = malloc(sizeof(Auto));
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

/*s: function ldobj(x86) */
void
ldobj(fdt f, long c, char *pn)
{
    /*s: [[ldobj()]] locals(x86) */
    // enum<as>, the opcode
    int o;
    Prog *p;
    /*x: [[ldobj()]] locals(x86) */
    Sym *h[NSYM];
    Sym *di;
    Sym *s;
    long ipc;
    bool skip;
    /*x: [[ldobj()]] locals(x86) */
    Prog *t;
    byte *stop;
    int v;
    ulong sig;
    /*x: [[ldobj()]] locals(x86) */
    byte *bloc;
    byte *bsize;
    int r;
    /*x: [[ldobj()]] locals(x86) */
    // array<string>, length used = files, extended every 16
    static char **filen;
    static int files = 0;
    char **nfilen;
    /*e: [[ldobj()]] locals(x86) */

    /*s: [[ldobj()]] grow filen if not enough space */
    if((files&15) == 0){
        nfilen = malloc((files+16)*sizeof(char*));
        memmove(nfilen, filen, files*sizeof(char*));
        free(filen);
        filen = nfilen;
    }
    /*e: [[ldobj()]] grow filen if not enough space */
    filen[files++] = strdup(pn);

    /*s: [[ldobj()]] bloc and bsize init */
    bsize = buf.ibuf;
    bloc = buf.ibuf;
    /*e: [[ldobj()]] bloc and bsize init */

    di = S;

// can come from AEND
newloop:
    version++;
    memset(h, 0, sizeof(h));
    histfrogp = 0;
    ipc = pc;
    skip = false;

loop:
    if(c <= 0)
        goto eof;

    /*s: [[ldobj()]] read if needed in loop:, adjust bloc and bsize */
    r = bsize - bloc;
    if(r < 100 && r < c) {		/* enough for largest prog */
        bsize = readsome(f, buf.ibuf, bloc, bsize, c);
        if(bsize == 0)
            goto eof;
        bloc = buf.ibuf;
        goto loop;
    }
    /*e: [[ldobj()]] read if needed in loop:, adjust bloc and bsize */

    // get the opcode
    o = bloc[0] | (bloc[1] << 8); // >>

    /*s: [[ldobj()]] sanity check opcode in range(x86) */
    if(o <= AXXX || o >= ALAST) {
        if(o < 0)
            goto eof;
        diag("%s: opcode out of range %d", pn, o);
        print("	probably not a .8 file\n");
        errorexit();
    }
    /*e: [[ldobj()]] sanity check opcode in range(x86) */

    /*s: [[ldobj()]] if ANAME or ASIGNAME(x86) */
    if(o == ANAME || o == ASIGNAME) {
        sig = 0;
        if(o == ASIGNAME) {
            sig = bloc[2] | (bloc[3]<<8) | (bloc[4]<<16) | (bloc[5]<<24);
            // >> >> >>
            bloc += 4;
            c -= 4;
        }
        stop = memchr(&bloc[4], 0, bsize-&bloc[4]);
        if(stop == nil){
            bsize = readsome(f, buf.ibuf, bloc, bsize, c);
            if(bsize == 0)
                goto eof;
            bloc = buf.ibuf;
            stop = memchr(&bloc[4], 0, bsize-&bloc[4]);
            if(stop == nil){
                fprint(2, "%s: name too long\n", pn);
                errorexit();
            }
        }

        v = bloc[2];	/* type */
        o = bloc[3];	/* sym */
        bloc += 4;
        c -= 4;

        r = 0;
        if(v == D_STATIC)
            r = version;
        s = lookup((char*)bloc, r);

        c -= &stop[1] - bloc;
        bloc = stop + 1;

        if(debug['S'] && r == 0)
            sig = 1729;
        if(sig != 0){
            if(s->sig != 0 && s->sig != sig)
                diag("incompatible type signatures %lux(%s) and %lux(%s) for %s", s->sig, filen[s->file], sig, pn, s->name);
            s->sig = sig;
            s->file = files-1;
        }

        if(debug['W'])
            print("	ANAME	%s\n", s->name);

        h[o] = s;
        if((v == D_EXTERN || v == D_STATIC) && s->type == 0)
            s->type = SXREF;
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
        goto loop;
    }
    /*e: [[ldobj()]] if ANAME or ASIGNAME(x86) */

    //TODO: factorize
    while(nhunk < sizeof(Prog))
        gethunk();
    p = (Prog*)hunk;
    nhunk -= sizeof(Prog);
    hunk += sizeof(Prog);

    p->as = o;
    p->line = bloc[2] | (bloc[3] << 8) | (bloc[4] << 16) | (bloc[5] << 24);
    p->back = 2;
    // >> >> >>

    r = zaddr(bloc+6, &p->from, h) + 6;
    r += zaddr(bloc+r, &p->to, h);

    bloc += r;
    c -= r;

    if(debug['W'])
        print("%P\n", p);

    switch(p->as) {
    /*s: [[ldobj()]] switch as cases(x86) */
    case ATEXT:
        if(curtext != P) {
            histtoauto();
            curtext->to.autom = curauto;
            curauto = 0;
        }
        skip = false;
        curtext = p;
        s = p->from.sym;
        if(s == S) {
            diag("%s: no TEXT symbol: %P", pn, p);
            errorexit();
        }
        if(s->type != 0 && s->type != SXREF) {
            if(p->from.scale & DUPOK) {
                skip = true;
                goto casdef;
            }
            diag("%s: redefinition: %s\n%P", pn, s->name, p);
        }
        s->type = STEXT;
        s->value = pc;

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
            etextp->pcond = p;
            etextp = p;
        }

        break;
    /*x: [[ldobj()]] switch as cases(x86) */
    case ADATA:
    data:
        //add_list(datap, edatap, p)
        if(edatap == P)
            datap = p;
        else
            edatap->link = p;
        edatap = p;
        p->link = P;
        break;
    /*x: [[ldobj()]] switch as cases(x86) */
    case ADYNT:
        if(p->to.sym == S) {
            diag("DYNT without a sym\n%P", p);
            break;
        }
        di = p->to.sym;
        p->from.scale = 4; // NOSPLIT?
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
        p->to.type = D_ADDR;
        p->to.index = D_EXTERN;
        goto data;
    /*x: [[ldobj()]] switch as cases(x86) */
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
        goto data;
    /*x: [[ldobj()]] switch as cases(x86) */
    case AGLOBL:
        s = p->from.sym;
        if(s->type == 0 || s->type == SXREF) {
            s->type = SBSS;
            s->value = 0;
        }
        if(s->type != SBSS) {
            diag("%s: redefinition: %s in %s",
                pn, s->name, TNAME);
            s->type = SBSS;
            s->value = 0;
        }
        if(p->to.offset > s->value)
            s->value = p->to.offset;
        break;
    /*x: [[ldobj()]] switch as cases(x86) */
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
        break;
    /*x: [[ldobj()]] switch as cases(x86) */
    case AEND:
        histtoauto();
        if(curtext != P)
            curtext->to.autom = curauto;
        curauto = 0;
        curtext = P;
        if(c)
            goto newloop;
        return;
    /*x: [[ldobj()]] switch as cases(x86) */
    case AGOK:
        diag("%s: GOK opcode in %s", pn, TNAME);
        pc++;
        break;
    /*x: [[ldobj()]] switch as cases(x86) */
    case AFMOVF:
    case AFADDF:
    case AFSUBF:
    case AFSUBRF:
    case AFMULF:
    case AFDIVF:
    case AFDIVRF:
    case AFCOMF:
    case AFCOMFP:
        if(skip)
            goto casdef;
        if(p->from.type == D_FCONST) {
            /* size sb 9 max */
            sprint(literal, "$%lux", ieeedtof(&p->from.ieee));
            s = lookup(literal, 0);
            if(s->type == 0) {
                s->type = SBSS;
                s->value = 4;
                t = prg();
                t->as = ADATA;
                t->line = p->line;
                t->from.type = D_EXTERN;
                t->from.sym = s;
                t->from.scale = 4;  // NOSPLIT?
                t->to = p->from;
                if(edatap == P)
                    datap = t;
                else
                    edatap->link = t;
                edatap = t;
                t->link = P;
            }
            p->from.type = D_EXTERN;
            p->from.sym = s;
            p->from.offset = 0;
        }
        goto casdef;
    /*x: [[ldobj()]] switch as cases(x86) */
    case AFMOVD:
    case AFADDD:
    case AFSUBD:
    case AFSUBRD:
    case AFMULD:
    case AFDIVD:
    case AFDIVRD:
    case AFCOMD:
    case AFCOMDP:
        if(skip)
            goto casdef;
        if(p->from.type == D_FCONST) {
            /* size sb 18 max */
            sprint(literal, "$%lux.%lux",
                p->from.ieee.l, p->from.ieee.h);
            s = lookup(literal, 0);
            if(s->type == 0) {
                s->type = SBSS;
                s->value = 8;
                t = prg();
                t->as = ADATA;
                t->line = p->line;
                t->from.type = D_EXTERN;
                t->from.sym = s;
                t->from.scale = 8;
                t->to = p->from;
                if(edatap == P)
                    datap = t;
                else
                    edatap->link = t;
                edatap = t;
                t->link = P;
            }
            p->from.type = D_EXTERN;
            p->from.sym = s;
            p->from.offset = 0;
        }
        goto casdef;
    /*e: [[ldobj()]] switch as cases(x86) */

    default:
    casdef:
        if(skip)
            nopout(p);

        if(p->to.type == D_BRANCH)
            p->to.offset += ipc;

        lastp->link = p;
        lastp = p;

        p->pc = pc;
        pc++;
    }
    goto loop;

eof:
    diag("truncated object file: %s", pn);
}
/*e: function ldobj(x86) */

/*s: function appendp(x86) */
Prog*
appendp(Prog *q)
{
    Prog *p;

    p = prg();
    p->link = q->link;
    q->link = p;
    p->line = q->line;
    return p;
}
/*e: function appendp(x86) */

/*s: function doprof1(x86) */
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

            //asm: DATA __mcount ??? array? why need to declare it here too?
            q->as = ADATA;
            q->from.type = D_EXTERN;
            q->from.offset = n*4;
            q->from.sym = s; // __mcount
            q->from.scale = 4;  // NOSPLIT?
            q->to = p->from;
            q->to.type = D_CONST;

            q = prg();
            q->line = p->line;
            q->pc = p->pc;

            q->link = p->link;
            p->link = q;
            p = q;

            //asm: ADDL 1, __mcount[n]?
            p->as = AADDL;
            p->from.type = D_CONST;
            p->from.offset = 1;
            p->to.type = D_EXTERN;
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

    //asm: DATA __mcount ???
    q->as = ADATA;
    q->from.type = D_EXTERN;
    q->from.sym = s;
    q->from.scale = 4;  // NOSPLIT?
    q->to.type = D_CONST;
    q->to.offset = n;

    s->type = SBSS;
    // 4 bytes counter for each functions
    s->value = n*4;
}
/*e: function doprof1(x86) */

/*s: function doprof2(x86) */
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
                // do not profile the profling function itself ...
                p->from.scale = NOPROF;
                ps2 = p;
            }
            if(p->from.sym == s4) {
                p->from.scale = NOPROF;
                ps4 = p;
            }
        }
    }

    for(p = firstp; p != P; p = p->link) {
        if(p->as == ATEXT) {
            curtext = p;

            /*s: [[doprof2()]] if NOPROF p(x86) */
            if(p->from.scale & NOPROF) {	/* dont profile */
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
            /*e: [[doprof2()]] if NOPROF p(x86) */

            /*
             * JMPL	profin
             */
            q = prg();
            q->line = p->line;
            q->pc = p->pc;
            q->link = p->link;

            /*s: [[doprof2()]] if embedded tracing ATEXT instrumentation(x86) */
            if(debug['e']){		/* embedded tracing */
                q2 = prg();
                p->link = q2;
                q2->link = q;

                q2->line = p->line;
                q2->pc = p->pc;

                q2->as = AJMP;
                q2->to.type = D_BRANCH;
                q2->to.sym = p->to.sym;
                q2->pcond = q->link;
            }
            /*e: [[doprof2()]] if embedded tracing ATEXT instrumentation(x86) */
             else
                p->link = q;
            p = q;
            //asm: CALL _profin
            p->as = ACALL;
            p->to.type = D_BRANCH;
            p->pcond = ps2;
            p->to.sym = s2;

        }else if(p->as == ARET) {
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
            q->link = p->link;

            p->link = q;

            /*
             * JAL	profout
             */
            //asm: CALL _profout
            p->as = ACALL;
            p->from = zprg.from;
            p->to = zprg.to;
            p->to.type = D_BRANCH;
            p->pcond = ps4;
            p->to.sym = s4;

            p = q;
        }
    }
}
/*e: function doprof2(x86) */

/*s: function nuxiinit(x86) */
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
        fnuxi8[i] = c;
        fnuxi8[i+4] = c+4;
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
/*e: function nuxiinit(x86) */

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
/*e: linkers/8l/obj.c */
