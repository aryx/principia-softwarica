/*s: mk/main.c */
#include	"mk.h"

/*s: constant MKFILE */
#define		MKFILE		"mkfile"
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
    Word *w;
    char *s, *temp;
    char *files[256], **f = files, **ff;
    bool sflag = false;
    int i;
    int tfd = -1;
    Biobuf tb;
    Bufblock *buf;
    Bufblock *whatif;

    /*
     *  start with a copy of the current environment variables
     *  instead of sharing them
     */

    Binit(&bout, STDOUT, OWRITE);
    buf = newbuf();
    whatif = nil;

    USED(argc);
    for(argv++; *argv && (**argv == '-'); argv++)
    {
        bufcpy(buf, argv[0], strlen(argv[0]));
        insert(buf, ' ');

        switch(argv[0][1]) {
        case 'a':
            aflag = true;
            break;
        case 'd':
            if(*(s = &argv[0][2]))
                while(*s) 
                 switch(*s++) {
                 case 'p':	debug |= D_PARSE; break;
                 case 'g':	debug |= D_GRAPH; break;
                 case 'e':	debug |= D_EXEC; break;
                }
            else
                debug = 0xFFFF;
            break;
        case 'e':
            explain = &argv[0][2];
            break;
        case 'f':
            if(*++argv == 0)
                badusage();
            *f++ = *argv;
            bufcpy(buf, argv[0], strlen(argv[0]));
            insert(buf, ' ');
            break;
        case 'i':
            iflag = true;
            break;
        case 'k':
            kflag = true;
            break;
        case 'n':
            nflag = true;
            break;
        case 's':
            sflag = true;
            break;
        case 't':
            tflag = true;
            break;
        case 'u':
            uflag = true;
            break;
        case 'w':
            if(whatif == nil)
                whatif = newbuf();
            else
                insert(whatif, ' ');
            if(argv[0][2])
                bufcpy(whatif, &argv[0][2], strlen(&argv[0][2]));
            else {
                if(*++argv == 0)
                    badusage();
                bufcpy(whatif, &argv[0][0], strlen(&argv[0][0]));
            }
            break;
        default:
            badusage();
        }
    }
    /*s: [[main()]] profiling */
    #ifdef	PROF
        {
            extern int etext();
            monitor(main, etext, buf, sizeof buf, 300);
        }
    #endif
    /*e: [[main()]] profiling */

    if(aflag)
        iflag = true;

    usage();
    syminit();
    initenv();
    usage();

    /*
        assignment args become null strings
    */
    temp = 0;
    for(i = 0; argv[i]; i++) if(utfrune(argv[i], '=')){
        bufcpy(buf, argv[i], strlen(argv[i]));
        insert(buf, ' ');
        if(tfd < 0){
            temp = maketmp();
            if(temp == 0) {
                perror("temp file");
                Exit();
            }
            if((tfd = create(temp, ORDWR, 0600)) < 0){
                perror(temp);
                Exit();
            }
            Binit(&tb, tfd, OWRITE);
        }
        Bprint(&tb, "%s\n", argv[i]);
        *argv[i] = 0;
    }
    if(tfd >= 0){
        Bflush(&tb);
        LSEEK(tfd, 0L, 0);
        parse("command line args", tfd, 1);
        remove(temp);
    }

    if (buf->current != buf->start) {
        buf->current--;
        insert(buf, '\0');
    }
    symlook("MKFLAGS", S_VAR, (void *) stow(buf->start));
    buf->current = buf->start;
    for(i = 0; argv[i]; i++){
        if(*argv[i] == 0) continue;
        if(i)
            insert(buf, ' ');
        bufcpy(buf, argv[i], strlen(argv[i]));
    }
    insert(buf, '\0');
    symlook("MKARGS", S_VAR, (void *) stow(buf->start));
    freebuf(buf);

    if(f == files){
        if(access(MKFILE, 4) == 0)
            parse(MKFILE, open(MKFILE, 0), 0);
    } else
        for(ff = files; ff < f; ff++)
            parse(*ff, open(*ff, 0), 0);

    /*s: [[main()]] if DEBUG(D_PARSE) */
    if(DEBUG(D_PARSE)){
        dumpw("default targets", target1);
        dumpr("rules", rules);
        dumpr("metarules", metarules);
        dumpv("variables");
    }
    /*e: [[main()]] if DEBUG(D_PARSE) */

    if(whatif){
        insert(whatif, '\0');
        timeinit(whatif->start);
        freebuf(whatif);
    }

    execinit();
    /* skip assignment args */
    while(*argv && (**argv == 0))
        argv++;

    catchnotes();
    if(*argv == 0){
        if(target1)
            for(w = target1; w; w = w->next)
                mk(w->s);
        else {
            fprint(STDERR, "mk: nothing to mk\n");
            Exit();
        }
    } else {
        if(sflag){
            for(; *argv; argv++)
                if(**argv)
                    mk(*argv);
        } else {
            Word *head, *tail, *t;

            /* fake a new rule with all the args as prereqs */
            tail = nil;
            t = nil;
            for(; *argv; argv++)
                if(**argv){
                    if(tail == 0)
                        tail = t = newword(*argv);
                    else {
                        t->next = newword(*argv);
                        t = t->next;
                    }
                }
            if(tail->next == 0)
                mk(tail->s);
            else {
                head = newword("command line arguments");
                addrules(head, tail, strdup(""), VIR, mkinline, 0);
                mk(head->s);
            }
        }
    }
    if(uflag)
        prusage();

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
