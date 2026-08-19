/*s: 5l/main.c */
#include	"l.h"

/*s: function usage, linker */
void
usage(void)
{
    print("usage: %s [-options] objects\n", argv0);
    errorexit();
}
/*e: function usage, linker */

/*s: function [[undef]] */
/// main -> <>
void
undef(void)
{
    int i;
    Sym *s;

    for(i=0; i<NHASH; i++)
        for(s = hash[i]; s != S; s = s->link)
            if(s->type == SXREF)
                diag("%s: not defined", s->name);
}
/*e: function [[undef]] */

/*s: function [[main]](arm) */
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
    Binit(&bso, STDOUT, OWRITE);
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
    // claude: raw kernel image, no header (see enum Headtype). HEADR=0;
    // the caller passes -T and -R (the bcm kernel uses -T0x80008000
    // -R4096). Matches kencc's case 6.
    case H_RAW:
        HEADR = 0;
        if(INITTEXT == -1)
            INITTEXT = 0;
        if(INITDAT == -1)
            INITDAT = 0;
        if(INITRND == -1)
            INITRND = 4;
        break;
    case H_ELF:	/* elf executable */
        HEADR = rnd(Ehdr32sz+3*Phdr32sz, 16);
        // claude: 0x8000 text base and page rounding like the kencc
        // lineage (the arm Linux convention; 4096+HEADR with -R4 gave
        // a layout that qemu-user tolerates but that diverges from the
        // proven kencc 5l output), see tests/s/variants
        if(INITTEXT == -1)
            INITTEXT = 0x8000+HEADR;
        if(INITDAT == -1)
            INITDAT = 0;
        if(INITRND == -1)
            INITRND = 4096;
        break;
    /*e: [[main()]] switch HEADTYPE cases(arm) */
    default:
        diag("unknown -H option");
        errorexit();
    }
    // claude: default the physical text address to the virtual one,
    // like the kencc 5l; it is emitted as p_paddr in the ELF program
    // headers (it stayed -1 = 0xffffffff without this)
    if (INITTEXTP == -1)
        INITTEXTP = INITTEXT;
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
    nuxiinit(); // endianess conversion tables
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
    //
    // claude: also record every -lXXX argument, and any plain filename
    // ending in ".a" (a library given as a direct path instead of
    // -lXXX -- e.g. principia's own kernel mkfiles pass $LIB this way),
    // into library[] (the same array loadlib() below rescans for
    // symbols later marked SXREF) as it goes by -- initdiv() (noop.c,
    // arm's software-divide helper lowering) needs to re-trigger a
    // library scan for _div/_divu/_mod/_modu after noops() runs, which
    // is *after* this loop and loadlib() below have both already
    // completed once; with neither form ever recorded anywhere,
    // initdiv()'s own loadlib() call had nothing to scan. Neither form
    // needs re-adding here for its *own* sake: objfile() already knows
    // how to resolve both (its own "-l" prefix handling, or just
    // open()ing the path directly), this only makes the same string
    // available for a *second* pass later. See
    // tests/c/regressions/arm_div_from_lib.c.
    while(*argv) {
        char *arg, *dot;

        arg = *argv;
        dot = strrchr(arg, '.');
        if(libraryp < nelem(library) &&
           ((arg[0] == '-' && arg[1] == 'l') ||
            (dot != nil && strcmp(dot, ".a") == 0)))
            library[libraryp++] = arg;
        objfile(*argv++);
    }
    /*s: [[main()]] load implicit libraries */
    if(!debug['l'])
        loadlib();
    /*e: [[main()]] load implicit libraries */

    // skip first empty instruction
    firstp = firstp->link;
    if(firstp == P)
        goto out;

    // claude: if the program uses arm's software divide/modulo helpers
    // (_div/_divu/_mod/_modu -- arm has no hardware integer divide),
    // resolve them now, while a library pull-in (loadlib(), inside
    // initdiv()) is still safe -- i.e. before patch()/dodata()/
    // follow()/noops() below, all of which assume every object is
    // already loaded. initdiv()'s own lazy call from inside noops()
    // (for programs that reach it directly, without going through this
    // early path) stays as a fallback that only diagnoses "undefined"
    // rather than actually loading anything at that point, since it's
    // too late to do so safely by then. See
    // tests/c/regressions/arm_div_from_lib.c.
    needsdiv();

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
    /* claude: match the kencc pass order: dodata(); follow(); noops();
     * span(). follow() (xfol) collapses the unreachable fall-through RET
     * the compiler emits after every function's real RET -- 5c emits
     * 'RET; RET' and relies on the linker to drop the dead one. principia
     * had dropped the follow() call entirely, so every function kept a
     * duplicate 'B (R14)' (~4 bytes each): the second of the two arm
     * linker mismatches (the other was BIG). follow() must run BEFORE
     * noops(), which rewrites ARET into B/MOVW and would hide the
     * terminal RET from xfol's detection. */
    dodata();
    follow();
    noops();
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
/*e: function [[main]](arm) */

/*e: 5l/main.c */
