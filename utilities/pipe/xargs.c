/*s: pipe/xargs.c */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */
#include <bio.h>

/*s: function [[usage]](xargs.c) */
void
usage(void)
{
    fprint(2, "usage: xargs [ -n lines ] [ -p procs ] args ...\n");
    exits("usage");
}
/*e: function [[usage]](xargs.c) */
/*s: function [[dowait]](xargs.c) */
void
dowait(void)
{
    while(waitpid() != -1)
        ;
}
/*e: function [[dowait]](xargs.c) */
/*s: function [[main]](xargs.c) */
void
main(int argc, char **argv)
{
    int lines, procs, i, j, run;
    char **nargv, **args, **p;
    static Biobuf bp;
    
    lines = 10;
    procs = 1;
    ARGBEGIN {
    case 'n': lines = atoi(EARGF(usage())); break;
    case 'p': procs = atoi(EARGF(usage())); break;
    default: usage();
    } ARGEND;
    if(argc < 1)
        usage();
    
    nargv = malloc(sizeof(char *) * (argc + lines + 1));
    if(nargv == nil)
        sysfatal("malloc: %r");
    memcpy(nargv, argv, sizeof(char *) * argc);
    args = nargv + argc;
    if(Binit(&bp, 0, OREAD) < 0)
        sysfatal("Binit: %r");
    //PAD: Blethal(&bp, nil); only in 9front
    atexit(dowait);
    for(j = 0, run = 1; run; j++){
        if(j >= procs)
            waitpid();
        memset(args, 0, sizeof(char *) * (lines + 1));
        for(i = 0; i < lines; i++)
            if((args[i] = Brdstr(&bp, '\n', 1)) == nil){
                if(i == 0)
                    exits(nil);
                run = 0;
                break;
            }
        switch(fork()){
        case -1:
            sysfatal("fork: %r");
        case 0:
            exec(*nargv, nargv);
            if(**nargv != '/' && strncmp(*nargv, "./", 2) != 0 &&
                    strncmp(*nargv, "../", 3) != 0){
                *nargv = smprint("/bin/%s", *nargv);
                exec(*nargv, nargv);
            }
            sysfatal("exec: %r");
        }
        for(p = args; *p; p++)
            free(*p);
    }
    exits(nil);
}
/*e: function [[main]](xargs.c) */
/*e: pipe/xargs.c */
