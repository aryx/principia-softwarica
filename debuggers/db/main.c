/*s: db/main.c */
/*
 * db - main command loop and error/interrupt handling
 */
#include "defs.h"
#include "fns.h"


extern	bool	executing;
extern	int	infile;
extern	int	eof;

int	alldigs(char*);
void	fault(void*, char*);

extern	char	*Ipath;
extern jmp_buf env;
extern char *errmsg;

/*s: function main (db/main.c) */
void
main(int argc, char **argv)
{
    /*s: [[main()]] locals (db) */
    char b1[100];
    char b2[100];
    /*x: [[main()]] locals (db) */
    char *s;
    /*x: [[main()]] locals (db) */
    char *name = nil;
    /*x: [[main()]] locals (db) */
    char *cpu, *p, *q;
    /*e: [[main()]] locals (db) */

    outputinit();

    ARGBEGIN{
    /*s: [[main()]] command line processing (db) */
    case 'I':
        s = ARGF();
        if(s == 0)
            dprint("missing -I argument\n");
        else
            Ipath = s;
        break;
    /*x: [[main()]] command line processing (db) */
    case 'w':
        wtflag = ORDWR;		/* suitable for open() */
        break;
    /*x: [[main()]] command line processing (db) */
    case 'm':
        name = ARGF();
        if(name == nil)
            dprint("missing -m argument\n");
        break;
    /*x: [[main()]] command line processing (db) */
    case 'k':
        kflag = true;
        break;
    /*e: [[main()]] command line processing (db) */
    }ARGEND

    if (argc > 0 && !alldigs(argv[0])) {
        symfil = argv[0];
        argv++;
        argc--;
    }
    /*s: [[main()]] if pid argument, attach to existing process */
    if(argc==1 && alldigs(argv[0])){

        pid = atoi(argv[0]);
        pcsactive = false;
        if (!symfil) {
            /*s: [[main()]] when pid argument, if kflag */
            if(kflag){
                cpu = getenv("cputype");
                if(cpu == nil){
                    cpu = "386";
                    dprint("$cputype not set; assuming %s\n", cpu);
                }
                p = getenv("terminal");
                if(p==nil || (p=strchr(p, ' '))==0 || p[1]==' ' || p[1]==0){
                    strcpy(b1, "/386/9pc");
                    dprint("missing or bad $terminal; assuming %s\n", b1);
                }else{
                    p++;
                    q = strchr(p, ' ');
                    if(q)
                        *q = '\0';
                    sprint(b1, "/%s/9%s", cpu, p);
    
                }
            }
            /*e: [[main()]] when pid argument, if kflag */
            else
                sprint(b1, "/proc/%s/text", argv[0]);
            symfil = b1;
        }
        sprint(b2, "/proc/%s/mem", argv[0]);
        corfil = b2;
    }
    /*e: [[main()]] if pid argument, attach to existing process */
    else if (argc > 0) {
        /*s: [[main()]] print usage and exit (db) */
        fprint(2, "Usage: db [-kw] [-m machine] [-I dir] ([symfile]|[pid])\n");
        exits("usage");
        /*e: [[main()]] print usage and exit (db) */
    }
 
   if (!symfil)
        symfil = "8.out";

    /*s: [[main()]] initialization before repl (db) */
    /*s: [[main()]] call notify */
    notify(fault);
    /*e: [[main()]] call notify */
    /*s: [[main()]] call setsym */
    setsym();
    /*e: [[main()]] call setsym */
    /*s: [[main()]] set dotmap */
    dotmap = dumbmap(-1);
    /*e: [[main()]] set dotmap */
    /*s: [[main()]] print binary architecture */
    /*s: [[main()]] if db -m and unknown machine */
    if (name && machbyname(name) == 0)
            dprint ("unknown machine %s", name);
    /*e: [[main()]] if db -m and unknown machine */
    dprint("%s binary\n", mach->name);
    /*e: [[main()]] print binary architecture */
    /*s: [[main()]] setjmp */
    if(setjmp(env) == 0){
        /*s: [[main()]] if setjmp == 0 and corfil */
        if (corfil) {
            setcor();	/* could get error */
            dprint("%s\n", machdata->excep(cormap, rget));
            printpc();
        }
        /*e: [[main()]] if setjmp == 0 and corfil */
    }
    setjmp(env);
    /*e: [[main()]] setjmp */
    /*s: [[main()]] just before repl */
    if (executing)
        delbp();
    executing = FALSE;
    /*e: [[main()]] just before repl */
    /*x: [[main()]] initialization before repl (db) */
    xargc = argc;
    /*e: [[main()]] initialization before repl (db) */

    // repl loop
    for (;;) {
        // clear output
        flushbuf();

        /*s: [[main()]] in loop, handle errmsg (db) */
        if (errmsg) {
            dprint(errmsg);
            printc('\n');
            errmsg = nil;
        }
        /*e: [[main()]] in loop, handle errmsg (db) */
        /*s: [[main()]] in loop, handle mkfault (db) */
        if (mkfault) {
            mkfault=0;
            printc('\n');
            prints(DBNAME);
        }
        /*e: [[main()]] in loop, handle mkfault (db) */

        // clear input
        clrinp();

        // go to next non whitespace char
        rdc();
        reread();
        /*s: [[main()]] in loop, if eof (db) */
        if (eof) {
            if (infile == STDIN)
                done(); // will exits()
            /*s: [[main()]] in loop, if eof, and if infile was not STDIN */
            iclose(-1, 0);
            eof = false;
            longjmp(env, 1);
            /*e: [[main()]] in loop, if eof, and if infile was not STDIN */
        }
        /*e: [[main()]] in loop, if eof (db) */

        command(nil, 0);

        reread();
        if (rdc() != '\n')
            error("newline expected");
    }
}
/*e: function main (db/main.c) */

/*s: function [[alldigs]] */
bool
alldigs(char *s)
{
    while(*s){
        if(*s<'0' || '9'<*s)
            return false;
        s++;
    }
    return true;
}
/*e: function [[alldigs]] */

/*s: function [[done]] */
void
done(void)
{
    if (pid)
        endpcs();
    exits(nil);
}
/*e: function [[done]] */


/*s: function [[fault]] */
/*
 * An interrupt occurred;
 * seek to the end of the current file
 * and remember that there was a fault.
 */
void
fault(void *a, char *s)
{
    USED(a);
    if(strncmp(s, "interrupt", 9) == 0){
        seek(infile, 0L, 2);
        mkfault++;
        noted(NCONT);
    }
    noted(NDFLT);
}
/*e: function [[fault]] */
/*e: db/main.c */
