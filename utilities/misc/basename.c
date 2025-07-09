/*s: misc/basename.c */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */

/*s: function [[main]](basename.c) */
void
main(int argc, char *argv[])
{
    char *pr;
    int n;
    bool dflag = false;

    if(argc>1 && strcmp(argv[1], "-d") == 0){
        --argc;
        ++argv;
        dflag = true;
    }
    if(argc < 2 || argc > 3){
        fprint(STDERR, "usage: basename [-d] string [suffix]\n");
        exits("usage");
    }
    pr = utfrrune(argv[1], '/');
    if(dflag){
        if(pr){
            *pr = '\0';
            print("%s\n", argv[1]);
            exits(nil);
        }
        // else
        print(".\n");
        exits(nil);
    }
    if(pr)
        pr++;
    else
        pr = argv[1];

    if(argc==3){
        n = strlen(pr)-strlen(argv[2]);
        if(n >= 0 && !strcmp(pr+n, argv[2]))
            pr[n] = '\0';
    }
    print("%s\n", pr);
    exits(nil);
}
/*e: function [[main]](basename.c) */
/*e: misc/basename.c */
