/*s: rc/main.c */
/*s: includes */
#include "rc.h"
#include "exec.h"
#include "fns.h"
#include "getflags.h"
#include "io.h"
/*e: includes */

// Rcmain and Isatty are back in plan9.c
extern char* Rcmain;
extern bool Isatty(fdt fd);

/*
 * get command line flags.
 * initialize keywords & traps.
 * get values from environment.
 * set $pid, $cflag, $*
 * fabricate bootstrap code and start it (*=(argv);. /usr/lib/rcmain $*)
 * start interpreting code
 */
/*s: function main (rc/exec.c) */
void main(int argc, char *argv[])
{
    /*s: [[main()]] locals */
    code bootstrap[17];
    /*x: [[main()]] locals */
    int i;
    /*x: [[main()]] locals */
    char *rcmain;
    /*x: [[main()]] locals */
    char num[12];
    /*e: [[main()]] locals */

    /*s: [[main()]] argc argv processing, modify flags */
    argc = getflags(argc, argv, "SsrdiIlxepvVc:1m:1[command]", 1);
    if(argc==-1)
        usage("[file [arg ...]]");
    /*x: [[main()]] argc argv processing, modify flags */
    if(argv[0][0]=='-')
        flag['l'] = flagset;
    /*x: [[main()]] argc argv processing, modify flags */
    if(flag['I'])
        flag['i'] = nil;
    else 
        if(flag['i']==nil && argc==1 && Isatty(STDIN)) 
           flag['i'] = flagset;
    /*e: [[main()]] argc argv processing, modify flags */

    /*s: [[main()]] initialisation */
    err = openfd(STDERR);
    /*x: [[main()]] initialisation */
    kinit();    // initialize keywords
    Trapinit(); // notify() function setup
    Vinit();    // read environment variables and add them in gvar
    /*x: [[main()]] initialisation */
    rcmain = flag['m'] ? flag['m'][0] : Rcmain; 
    /*x: [[main()]] initialisation */
    mypid = getpid();
    inttoascii(num, mypid);
    setvar("pid", newword(num, (word *)nil));
    /*x: [[main()]] initialisation */
    setvar("rcname", newword(argv[0], (word *)nil));
    /*x: [[main()]] initialisation */
    setvar("cflag", flag['c']? newword(flag['c'][0], (word *)nil) : (word *)nil);
    /*e: [[main()]] initialisation */
    /*s: [[main()]] initialize [[boostrap]] */
    memset(bootstrap, 0, sizeof bootstrap);

    i = 0;
    bootstrap[i++].i=1;
    // runq->argv is populated with the arguments to rc
    // we just need to add '*=(argv)'
    bootstrap[i++].f = Xmark;
      bootstrap[i++].f = Xword;
      bootstrap[i++].s="*";
    bootstrap[i++].f = Xassign; // will pop_list() x2

    bootstrap[i++].f = Xmark;
      bootstrap[i++].f = Xmark;
        bootstrap[i++].f = Xword;
        bootstrap[i++].s="*";
      bootstrap[i++].f = Xdol; // will pop_list()
      bootstrap[i++].f = Xword;
      bootstrap[i++].s = rcmain;
      bootstrap[i++].f = Xword;
      bootstrap[i++].s=".";
    bootstrap[i++].f = Xsimple; // will pop_list()

    bootstrap[i++].f = Xexit;

    bootstrap[i].i = 0;
    /*e: [[main()]] initialize [[boostrap]] */
    /*s: [[main()]] initialize [[runq]] with bootstrap code */
    start(bootstrap, 1, (var *)nil);
    /*x: [[main()]] initialize [[runq]] with bootstrap code */
    runq->cmdfd = openfd(STDIN); // reading from stdin
    runq->cmdfile = "<stdin>";
    runq->iflag = flag['i']? true : false;// interactive mode; will print a prompt
    /*e: [[main()]] initialize [[runq]] with bootstrap code */
    /*s: [[main()]] initialize [[runq->argv]] */
    /* prime bootstrap argv */
    pushlist();
    argv0 = strdup(argv[0]);
    for(i = argc-1; i!=0; --i) 
        pushword(argv[i]);
    /*e: [[main()]] initialize [[runq->argv]] */

    /*s: [[main()]] interpreter loop */
    for(;;){
        /*s: [[main()]] debug runq in interpreter loop */
        if(flag['r'])
            pfnc(err, runq);
        /*e: [[main()]] debug runq in interpreter loop */

        runq->pc++;
        (*runq->code[runq->pc-1].f)();

        /*s: [[main()]] handing trap if necessary in interpreter loop */
        if(ntrap)
            dotrap();
        /*e: [[main()]] handing trap if necessary in interpreter loop */
    }
    /*e: [[main()]] interpreter loop */
}
/*e: function main (rc/exec.c) */

/*e: rc/main.c */
