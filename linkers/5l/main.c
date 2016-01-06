/*s: linkers/5l/main.c */
#include	"l.h"

/*s: global thechar */
char	thechar;
/*e: global thechar */
/*s: global thestring */
char*	thestring;
/*e: global thestring */

/*s: function usage, linker */
void
usage(void)
{
    print("usage: %s [-options] objects", argv0);
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
        break;
    /*x: [[main()]] command line processing(arm) */
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
    /*x: [[main()]] command line processing(arm) */
    case 'R':
        a = ARGF();
        if(a)
            INITRND = atolwhex(a);
        break;
    /*x: [[main()]] command line processing(arm) */
    case 'E':
        a = ARGF();
        if(a)
            INITENTRY = a;
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

    /*s: [[main()]] initialize globals(arm) */
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
    if(HEADTYPE == -1)
        HEADTYPE = H_PLAN9;
    switch(HEADTYPE) {
    /*s: [[main()]] switch HEADTYPE cases(arm) */
    case H_PLAN9:
        HEADR = 32L;
        if(INITTEXT == -1)
            INITTEXT = 4096+32; // 1 page + a.out header = 4128
        if(INITDAT == -1)
            INITDAT = 0;
        if(INITRND == -1)
            INITRND = 4096; // 1 page
        break;
    /*x: [[main()]] switch HEADTYPE cases(arm) */
    case H_ELF:	/* elf executable */
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
    /*s: [[main()]] sanity check INITXXX */
    if(INITDAT != 0 && INITRND != 0)
        print("warning: -D0x%lux is ignored because of -R0x%lux\n",
            INITDAT, INITRND);
    /*e: [[main()]] sanity check INITXXX */
    DBG("HEADER = -H%d -T0x%lux -D0x%lux -R0x%lux\n",
            HEADTYPE, INITTEXT, INITDAT, INITRND);
    /*e: [[main()]] set HEADTYPE, INITTEXT, INITDAT, etc */
    /*s: [[main()]] set INITENTRY */
    if(INITENTRY == nil) {
        INITENTRY = "_main";
        /*s: [[main()]] adjust INITENTRY if profiling */
        if(debug['p'])
            INITENTRY = "_mainp";
        /*e: [[main()]] adjust INITENTRY if profiling */
    }
    /*s: [[main()]] if rare condition do not set SXREF for INITENTRY, else */
    if(debug['l']) {}
    else
    /*x: [[main()]] if rare condition do not set SXREF for INITENTRY, else */
    if(*INITENTRY >= '0' && *INITENTRY <= '9') {}
    else
    /*e: [[main()]] if rare condition do not set SXREF for INITENTRY, else */
      lookup(INITENTRY, 0)->type = SXREF;
    /*e: [[main()]] set INITENTRY */
    /*x: [[main()]] initialize globals(arm) */
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
    nuxiinit(); // ???
    /*x: [[main()]] initialize globals(arm) */
    buildop();
    /*x: [[main()]] initialize globals(arm) */
    cbp = buf.obuf;
    cbc = sizeof(buf.obuf);
    /*e: [[main()]] initialize globals(arm) */

    cout = create(outfile, 1, 0775);
    /*s: [[main()]] sanity check cout */
    if(cout < 0) {
        diag("cannot create %s: %r", outfile);
        errorexit();
    }
    /*e: [[main()]] sanity check cout */

    // ------ main functions  ------
    /*s: [[main()]] cout is ready, LET'S GO(arm) */
    // first empty instruction
    firstp = prg();
    lastp = firstp;

    // Loading (populates firstp, datap, and hash)
    while(*argv)
        objfile(*argv++);
    /*s: [[main()]] load implicit libraries */
    if(!debug['l'])
        loadlib();
    /*e: [[main()]] load implicit libraries */

    // skip first empty instruction
    firstp = firstp->link;
    if(firstp == P)
        goto out;

    // Resolving
    /*s: [[main()]] resolving phase */
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
    noops();

    dodata();
    dotext();
    /*e: [[main()]] resolving phase */

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

/*e: linkers/5l/main.c */
