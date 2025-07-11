/*s: compare/cmp.c */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */
#include <ctype.h>

/*s: constant [[BUF]](cmp.c) */
#define     BUF     65536
/*e: constant [[BUF]](cmp.c) */

/*s: global flags (cmp.c) */
// print nothing for differing files (shutup) but set exit status
bool sflag = false;
// print byte number (decimal) and differing bytes (hexa)
bool lflag = false;
// print the line number of the first differing bytes
bool Lflag = false;
/*e: global flags (cmp.c) */

// forward decl
static void usage(void);

/*s: function [[seekoff]](cmp.c) */
char **
seekoff(fdt fd, char *name, char **argv)
{
    vlong o;

    if(*argv){
        if (!isascii(**argv) || !isdigit(**argv))
            usage();
        o = strtoll(*argv++, nil, 0);
        if(seek(fd, o, SEEK__START) < 0){
            if(!sflag) fprint(STDERR, "cmp: %s: seek by %lld: %r\n",
                name, o);
            exits("seek");
        }
    }
    return argv;
}
/*e: function [[seekoff]](cmp.c) */

/*s: function [[main]](cmp.c) */
void
main(int argc, char *argv[])
{
    /*s: [[main]] locals (cmp.c) */
    char *name1, *name2;
    fdt f1, f2;
    uchar buf1[BUF], buf2[BUF];
    uchar *b1s, *b1e, *b2s, *b2e;
    /*x: [[main]] locals (cmp.c) */
    int n, i;
    /*x: [[main]] locals (cmp.c) */
    uchar *p, *q;
    /*x: [[main]] locals (cmp.c) */
    // ??
    vlong nc = 1;
    // line
    vlong l = 1;
    /*e: [[main]] locals (cmp.c) */

    ARGBEGIN{
    case 's':   sflag = true; break;
    case 'l':   lflag = true; break;
    case 'L':   Lflag = true; break;
    default:    usage();
    }ARGEND

    if(argc < 2 || argc > 4)
        usage();

    if((f1 = open(name1 = *argv++, OREAD)) == -1){
        if(!sflag) perror(name1);
        exits("open");
    }
    if((f2 = open(name2 = *argv++, OREAD)) == -1){
        if(!sflag) perror(name2);
        exits("open");
    }
    argv = seekoff(f1, name1, argv);
    argv = seekoff(f2, name2, argv);
    if(*argv)
        usage();

    b1s = b1e = buf1;
    b2s = b2e = buf2;

    for(;;){
        if(b1s >= b1e){
            if(b1s >= &buf1[BUF])
                b1s = buf1;
            n = read(f1, b1s,  &buf1[BUF] - b1s);
            b1e = b1s + n;
        }
        if(b2s >= b2e){
            if(b2s >= &buf2[BUF])
                b2s = buf2;
            n = read(f2, b2s,  &buf2[BUF] - b2s);
            b2e = b2s + n;
        }
        n = b2e - b2s;
        if(n > b1e - b1s)
            n = b1e - b1s;
        if(n <= 0)
            break;

        if(memcmp((void *)b1s, (void *)b2s, n) != 0){
            if(sflag)
                exits("differ");
            for(p = b1s, q = b2s, i = 0; i < n; p++, q++, i++) {
                if(*p == '\n')
                    l++;
                if(*p != *q){
                    if(!lflag){
                        print("%s %s differ: char %lld",
                            name1, name2, nc+i);
                        print(Lflag?" line %lld\n":"\n", l);
                        exits("differ");
                    }
                    print("%6lld 0x%.2x 0x%.2x\n", nc+i, *p, *q);
                }
            }
        }       
        if(Lflag)
            for(p = b1s; p < b1e;)
                if(*p++ == '\n')
                    l++;
        nc += n;
        b1s += n;
        b2s += n;
    } // end for (;;)

    if (b1e - b1s < 0 || b2e - b2s < 0) {
        if (!sflag) {
            if (b1e - b1s < 0)
                print("error on %s after %lld bytes\n",
                    name1, nc-1);
            if (b2e - b2s < 0)
                print("error on %s after %lld bytes\n",
                    name2, nc-1);
        }
        exits("read error");
    }

    // files are the same, exit 0
    if(b1e - b1s == b2e - b2s)
        exits(nil);

    if(!sflag)
        print("EOF on %s after %lld bytes\n",
            (b1e - b1s > b2e - b2s)? name2 : name1, nc-1);
    exits("EOF");
}
/*e: function [[main]](cmp.c) */

/*s: function [[usage]](cmp.c) */
static void
usage(void)
{
    print("usage: cmp [-lLs] file1 file2 [offset1 [offset2] ]\n");
    exits("usage");
}
/*e: function [[usage]](cmp.c) */
/*e: compare/cmp.c */
