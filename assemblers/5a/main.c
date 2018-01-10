/*s: assemblers/5a/main.c */
#include "a.h"

// forward decls
int		assemble(char*);
void	cclean(void);

/*s: function main(arm) */
void
main(int argc, char *argv[])
{
    errorn err;
    /*s: [[main()]] locals */
    char *p;
    /*x: [[main()]] locals */
    int nout, nproc, status;
    int i, c;
    /*e: [[main()]] locals */

    thechar = '5';
    thestring = "arm";

    cinit();
    /*s: [[main()]] remaining initializations */
    include[ninclude++] = ".";
    /*x: [[main()]] remaining initializations */
    memset(debug, false, sizeof(debug));
    /*e: [[main()]] remaining initializations */

    ARGBEGIN {
    /*s: [[main()]] command line processing */
    case 'o':
        outfile = ARGF();
        break;
    /*x: [[main()]] command line processing */
    case 'I':
        p = ARGF();
        setinclude(p);
        break;
    /*x: [[main()]] command line processing */
    case 'D':
        p = ARGF();
        if(p)
            Dlist[nDlist++] = p;
        break;
    /*x: [[main()]] command line processing */
    default:
        c = ARGC();
        if(c >= 0 || c < sizeof(debug))
            debug[c] = true;
        break;
    /*e: [[main()]] command line processing */
    } ARGEND

    if(*argv == '\0') {
        print("usage: %ca [-options] file.s\n", thechar);
        errorexit();
    }

    /*s: [[main()]] multiple files handling */
    if(argc > 1) {
        nproc = 1;
        if(p = getenv("NPROC"))
            nproc = atol(p);	/* */
        c = 0;
        nout = 0;
        for(;;) {
            while(nout < nproc && argc > 0) {
                i = fork();
                if(i < 0) {
                    i = mywait(&status);
                    if(i < 0)
                        errorexit();
                    if(status)
                        c++;
                    nout--;
                    continue;
                }
                if(i == 0) {
                    print("%s:\n", *argv);
                    if(assemble(*argv))
                        errorexit();
                    exits(nil);
                }
                nout++;
                argc--;
                argv++;
            }
            i = mywait(&status);
            if(i < 0) {
                if(c)
                    errorexit();
                exits(nil);
            }
            if(status)
                c++;
            nout--;
        }
    }
    /*e: [[main()]] multiple files handling */
    // else

    err = assemble(argv[0]);
    if(err > 0)
        errorexit();
    else
        exits(nil);
}
/*e: function main(arm) */

/*s: function assemble */
errorn
assemble(char *infile)
{
    fdt of; // outfile
    /*s: [[assemble()]] locals */
    char ofile[100];
    /*x: [[assemble()]] locals */
    char *p;
    /*x: [[assemble()]] locals */
    char incfile[20];
    /*x: [[assemble()]] locals */
    int i;
    /*e: [[assemble()]] locals */

    /*s: [[assemble()]] set p to basename(infile) and adjust include */
    // p = basename(infile)
    strcpy(ofile, infile);
    p = utfrrune(ofile, '/');
    if(p) {
        *p++ = '\0';
        /*s: [[assemble()]] adjust first entry in include with dirname infile */
        include[0] = ofile;
        /*e: [[assemble()]] adjust first entry in include with dirname infile */
    } else
        p = ofile;
    /*e: [[assemble()]] set p to basename(infile) and adjust include */
    if(outfile == nil) {
        /*s: [[assemble()]] set outfile to {basename(infile)}.{thechar} */
        // outfile =  p =~ s/.s/.5/;
        outfile = p;
        if(outfile){
            p = utfrrune(outfile, '.');
            if(p)
                if(p[1] == 's' && p[2] == '\0')
                    p[0] = '\0';
            p = utfrune(outfile, '\0');
            p[0] = '.';
            p[1] = thechar;
            p[2] = '\0';
        } else
            outfile = "/dev/null";
        /*e: [[assemble()]] set outfile to {basename(infile)}.{thechar} */
    }
    /*s: [[assemble()]] setinclude("/{thestring}/include") or use INCLUDE */
    p = getenv("INCLUDE");
    if(p) {
        setinclude(p);
    } else {
         sprint(incfile,"/%s/include", thestring);
         setinclude(strdup(incfile));
    }
    /*e: [[assemble()]] setinclude("/{thestring}/include") or use INCLUDE */

    of = mycreat(outfile, 0664);
    /*s: [[assemble()]] sanity check [[of]] */
    if(of < 0) {
        yyerror("%ca: cannot create %s", thechar, outfile);
        errorexit();
    }
    /*e: [[assemble()]] sanity check [[of]] */
    Binit(&obuf, of, OWRITE);

    // Pass 1
    pass = 1;
    pinit(infile);
    /*s: [[assemble()]] init Dlist after pinit */
    for(i=0; i<nDlist; i++)
            dodefine(Dlist[i]);
    /*e: [[assemble()]] init Dlist after pinit */
    yyparse(); // calls outcode(), which does almost nothing when pass == 1
    /*s: [[assemble()]] sanity check nerrors */
    if(nerrors) {
        cclean();
        return nerrors;
    }
    /*e: [[assemble()]] sanity check nerrors */

    // Pass 2
    pass = 2;
    outhist(); // output file/line history information in object file
    pinit(infile);
    /*s: [[assemble()]] init Dlist after pinit */
    for(i=0; i<nDlist; i++)
            dodefine(Dlist[i]);
    /*e: [[assemble()]] init Dlist after pinit */
    yyparse(); // calls outcode() which now does things

    cclean();
    return nerrors;
}
/*e: function assemble */

/*s: function cclean(arm) */
/// main -> assemble -> <>
void
cclean(void)
{

    outcode(AEND, Always, &nullgen, R_NONE, &nullgen);
    Bflush(&obuf);
}
/*e: function cclean(arm) */

/*e: assemblers/5a/main.c */
