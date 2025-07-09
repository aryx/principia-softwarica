/*s: files/mtime.c */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */

/*s: function [[usage]](mtime.c) */
void
usage(void)
{
    fprint(2, "usage: mtime file...\n");
    exits("usage");
}
/*e: function [[usage]](mtime.c) */
/*s: function [[main]](mtime.c) */
void
main(int argc, char **argv)
{
    int errors, i;
    Dir *d;

    ARGBEGIN{
    default:
        usage();
    }ARGEND

    errors = 0;
    for(i=0; i<argc; i++){
        if((d = dirstat(argv[i])) == nil){
            fprint(2, "stat %s: %r\n", argv[i]);
            errors = 1;
        }else{
            print("%11lud %s\n", d->mtime, argv[i]);
            free(d);
        }
    }
    exits(errors ? "errors" : nil);
}
/*e: function [[main]](mtime.c) */
/*e: files/mtime.c */
