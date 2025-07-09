/*s: pipe/tee.c */
/*
 * tee-- pipe fitting
 */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */

/*s: global flags(tee.c) */
// append mode
bool aflag;
// deprecated
bool uflag;
/*e: global flags(tee.c) */
/*s: global [[openf]](tee.c) */
fdt *openf;
/*e: global [[openf]](tee.c) */
/*s: global [[in]](tee.c) */
// read buffer
char in[8192];
/*e: global [[in]](tee.c) */

bool intignore(void*, char*);

/*s: function [[main]](tee.c) */
void
main(int argc, char **argv)
{
    int i;
    int r, n;

    ARGBEGIN {
    case 'a':
        aflag = true;
        break;

    case 'i':
        atnotify(intignore, true); // register
        break;

    case 'u':
        uflag = true;
        /* uflag is ignored and undocumented; it's a relic from Unix */
        break;

    default:
        fprint(STDERR, "usage: tee [-ai] [file ...]\n");
        exits("usage");
    } ARGEND

    openf = malloc((1+argc)*sizeof(int));
    if(openf == nil)
        sysfatal("out of memory: %r");

    n = 0;
    while(*argv) {
        if(aflag) {
            openf[n] = open(argv[0], OWRITE);
            if(openf[n] < 0)
                openf[n] = create(argv[0], OWRITE, 0666);
            seek(openf[n], 0L, SEEK__END);
        } else
            openf[n] = create(argv[0], OWRITE, 0666);
        if(openf[n] < 0) {
            fprint(STDERR, "tee: cannot open %s: %r\n", argv[0]);
        } else
            n++;
        argv++;
    }
    openf[n++] = STDOUT;

    for(;;) {
        r = read(STDIN, in, sizeof in);
        if(r <= 0)
            exits(nil);
        for(i=0; i<n; i++)
            write(openf[i], in, r);
    }
}
/*e: function [[main]](tee.c) */
/*s: function [[intignore]](tee.c) */
bool
intignore(void *a, char *msg)
{
    USED(a);
    if(strcmp(msg, "interrupt") == 0)
        return true;
    return false;
}
/*e: function [[intignore]](tee.c) */
/*e: pipe/tee.c */
