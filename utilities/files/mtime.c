/*s: files/mtime.c */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */

/*s: function [[usage]](mtime.c) */
void
usage(void)
{
    fprint(STDERR, "usage: mtime file...\n");
    exits("usage");
}
/*e: function [[usage]](mtime.c) */
/*s: function [[main]](mtime.c) */
void
main(int argc, char **argv)
{
    bool errors = false;
    int i;
    Dir *d;

    ARGBEGIN{
    default:
        usage();
    }ARGEND

    for(i=0; i<argc; i++){
        if((d = dirstat(argv[i])) == nil){
            fprint(STDERR, "stat %s: %r\n", argv[i]);
            errors = true;
        }else{
            print("%11lud %s\n", d->mtime, argv[i]);
            free(d);
        }
    }
    exits(errors ? "errors" : nil);
}
/*e: function [[main]](mtime.c) */
/*e: files/mtime.c */
