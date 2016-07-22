/*s: mk/main.c */
#include	"mk.h"

/*s: constant MKFILE */
#define	MKFILE		"mkfile"
/*e: constant MKFILE */

/*s: global version */
static char *version = "@(#)mk general release 4 (plan 9)";
/*e: global version */

// see also globals.c

/*s: global uflag */
bool uflag = false;
/*e: global uflag */

void badusage(void);

#ifdef	PROF
/*s: global buf */
short buf[10000];
/*e: global buf */
#endif

/*s: function main */
void
main(int argc, char **argv)
{
    /*s: [[main()]] locals */
    Word *w;
    char *s;
    int i;
    /*x: [[main()]] locals */
    char *temp = nil;
    fdt tfd = -1;
    Biobuf tb;
    /*x: [[main()]] locals */
    char *files[256];
    char **f = files;
    /*x: [[main()]] locals */
    char **ff;
    /*x: [[main()]] locals */
    bool sflag = false;
    /*x: [[main()]] locals */
    Bufblock *whatif = nil;
    /*x: [[main()]] locals */
    Bufblock *buf = newbuf();
    /*e: [[main()]] locals */

    // Initializing

    /*s: [[main()]] initializations */
    /*
     *  start with a copy of the current environment variables
     *  instead of sharing them
     */
    //??? for execinit?? for initenv/

    Binit(&bout, STDOUT, OWRITE);
    /*s: [[main()]] argv processing part 1, -xxx */
    USED(argc);
    for(argv++; *argv && (**argv == '-'); argv++)
    {
        /*s: [[main()]] add [[argv[0]]] in [[buf]] */
        bufcpy(buf, argv[0], strlen(argv[0]));
        insert(buf, ' ');
        /*e: [[main()]] add [[argv[0]]] in [[buf]] */

        switch(argv[0][1]) {
        /*s: [[main()]] -xxx switch cases */
        case 'f':
            if(*++argv == nil)
                badusage();
            *f++ = *argv;
            /*s: [[main()]] add [[argv[0]]] in [[buf]] */
            bufcpy(buf, argv[0], strlen(argv[0]));
            insert(buf, ' ');
            /*e: [[main()]] add [[argv[0]]] in [[buf]] */
            break;
        /*x: [[main()]] -xxx switch cases */
        case 's':
            sflag = true;
            break;
        /*x: [[main()]] -xxx switch cases */
        case 'e':
            explain = true;
            break;
        /*x: [[main()]] -xxx switch cases */
        case 'n':
            nflag = true;
            break;
        /*x: [[main()]] -xxx switch cases */
        case 'w':
            if(whatif == nil)
                whatif = newbuf();
            else
                insert(whatif, ' ');
            if(argv[0][2])
                bufcpy(whatif, &argv[0][2], strlen(&argv[0][2]));
            else {
                if(*++argv == '\0')
                    badusage();
                bufcpy(whatif, &argv[0][0], strlen(&argv[0][0]));
            }
            break;
        /*x: [[main()]] -xxx switch cases */
        case 'u':
            uflag = true;
            break;
        /*x: [[main()]] -xxx switch cases */
        case 'i':
            iflag = true;
            break;
        /*x: [[main()]] -xxx switch cases */
        case 't':
            tflag = true;
            break;
        /*x: [[main()]] -xxx switch cases */
        case 'a':
            aflag = true;
            iflag = true;
            break;
        /*x: [[main()]] -xxx switch cases */
        case 'd':
            if(*(s = &argv[0][2]))
                while(*s) 
                 switch(*s++) {
                 case 'p':	debug |= D_PARSE; break;
                 case 'g':	debug |= D_GRAPH; break;
                 case 'e':	debug |= D_EXEC; break;
                }
            else
                debug = 0xFFFF; // D_PARSE | D_GRAPH | D_EXEC
            break;
        /*x: [[main()]] -xxx switch cases */
        case 'k':
            kflag = true;
            break;
        /*e: [[main()]] -xxx switch cases */
        default:
            badusage();
        }
    }
    /*e: [[main()]] argv processing part 1, -xxx */
    /*s: [[main()]] setup optional profiling */
    #ifdef	PROF
        {
            extern int etext();
            monitor(main, etext, buf, sizeof buf, 300);
        }
    #endif
    /*e: [[main()]] setup optional profiling */
    usage();
    syminit();
    initenv();
    usage();
    /*s: [[main()]] argv processing part 2, xxx=yyy */
    /*
     *   assignment args become null strings
     */
    for(i = 0; argv[i]; i++) 
      if(utfrune(argv[i], '=')){
        /*s: [[main()]] add [[argv[i]]] in [[buf]] */
        bufcpy(buf, argv[i], strlen(argv[i]));
        insert(buf, ' ');
        /*e: [[main()]] add [[argv[i]]] in [[buf]] */

        /*s: [[main()]] create temporary file if not exist yet */
        if(tfd < 0){
            temp = maketmp();
            if(temp == nil) {
                perror("temp file");
                Exit();
            }
            tfd = create(temp, ORDWR, 0600);
            if(tfd < 0){
                perror(temp);
                Exit();
            }
            Binit(&tb, tfd, OWRITE);
        }
        /*e: [[main()]] create temporary file if not exist yet */
        Bprint(&tb, "%s\n", argv[i]);
        *argv[i] = '\0';
      }

    if(tfd >= 0){
        Bflush(&tb);
        seek(tfd, 0L, SEEK__START);
        parse("<command line args>", tfd, true);
        remove(temp);
    }
    /*e: [[main()]] argv processing part 2, xxx=yyy */
    /*s: [[main()]] set variables for recursive mk */
    /*s: [[main()]] set MKFLAGS variable */
    if (buf->current != buf->start) {
        buf->current--;
        insert(buf, '\0');
    }
    symlook("MKFLAGS", S_VAR, (void*) stow(buf->start));
    /*e: [[main()]] set MKFLAGS variable */
    /*s: [[main()]] set MKARGS variable */
    buf->current = buf->start;
    for(i = 0; argv[i]; i++){
        if(*argv[i] == '\0') 
            continue;
        if(i)
            insert(buf, ' ');
        bufcpy(buf, argv[i], strlen(argv[i]));
    }
    insert(buf, '\0');
    symlook("MKARGS", S_VAR, (void *) stow(buf->start));

    freebuf(buf);
    /*e: [[main()]] set MKARGS variable */
    /*e: [[main()]] set variables for recursive mk */
    /*e: [[main()]] initializations */

    // Parsing

    /*s: [[main()]] parsing mkfile, call parse() */
    if(f == files){
        if(access(MKFILE, AREAD) == 0)
            parse(MKFILE, open(MKFILE, OREAD), false);
    } else
        for(ff = files; ff < f; ff++)
            parse(*ff, open(*ff, OREAD), false);
    /*s: [[main()]] if DEBUG(D_PARSE) */
    if(DEBUG(D_PARSE)){
        dumpw("default targets", target1);
        dumpr("rules", rules);
        dumpr("metarules", metarules);
        dumpv("variables");
    }
    /*e: [[main()]] if DEBUG(D_PARSE) */
    /*e: [[main()]] parsing mkfile, call parse() */

    // Querying (optional)

    /*s: [[main()]] whatif optional setup */
    if(whatif){
        insert(whatif, '\0');
        timeinit(whatif->start);
        freebuf(whatif);
    }
    /*e: [[main()]] whatif optional setup */

    // Building

    /*s: [[main()]] initializations before building */
    execinit();

    /*s: [[main()]] argv processing part 3, skip xxx=yyy */
    /* skip assignment args */
    while(*argv && (**argv == '\0'))
        argv++;
    /*e: [[main()]] argv processing part 3, skip xxx=yyy */

    catchnotes();
    /*e: [[main()]] initializations before building */
    /*s: [[main()]] building the targets, call mk() */
    if(*argv == nil){
        /*s: [[main()]] when no target arguments */
        if(target1)
            for(w = target1; w; w = w->next)
                // The call!
                mk(w->s);
        else {
            fprint(STDERR, "mk: nothing to mk\n");
            Exit();
        }
        /*e: [[main()]] when no target arguments */
    } else {
        /*s: [[main()]] if sequential mode and target arguments given */
        if(sflag){
            for(; *argv; argv++)
                if(**argv)
                    mk(*argv);
        }
        /*e: [[main()]] if sequential mode and target arguments given */
        else {
           /*s: [[main()]] parallel mode and target arguments given */
           Word *head, *tail;
           Word *t;

           /* fake a new rule with all the args as prereqs */
           tail = nil;
           t = nil;
           for(; *argv; argv++)
               if(**argv){
                   // add_list(newword(*argv), t)
                   if(tail == nil)
                       tail = t = newword(*argv);
                   else {
                       t->next = newword(*argv);
                       t = t->next;
                   }
               }
           if(tail->next == nil)
               // a single target argument
               mk(tail->s);
           else {
               head = newword("<command line arguments>");
               addrules(head, tail, strdup(""), VIR, mkinline, nil);
               mk(head->s);
           }
           /*e: [[main()]] parallel mode and target arguments given */
        }
    }
    /*e: [[main()]] building the targets, call mk() */

    // Reporting (optional)

    /*s: [[main()]] print profiling stats if uflag */
    if(uflag)
        prusage();
    /*e: [[main()]] print profiling stats if uflag */

    // Exiting

    exits(nil);
}
/*e: function main */

/*s: function badusage */
void
badusage(void)
{

    fprint(STDERR, "Usage: mk [-f file] [-n] [-a] [-e] [-t] [-k] [-i] [-d[egp]] [targets ...]\n");
    Exit();
}
/*e: function badusage */
/*e: mk/main.c */
