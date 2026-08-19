/*s: misc/strings.c */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */
#include    <bio.h>

Biobuf  *fin;
Biobuf  fout;

#define MINSPAN     6       /* Min characters in string (default) */
#define BUFSIZE     70

void stringit(char *);
// claude: renamed from isprint(Rune) -- collided with include/str/
// ascii.h's own `#define isprint(c) ...` byte-oriented ctype macro
// (already reached via libc.h), which textually rewrote this file's
// own declaration/definition/call sites into nonsense ("syntax error,
// last name: Rune" was the macro-mangled result, not a real parser
// bug). Matches utf.h's own isalpharune()/isdigitrune() naming for
// exactly this "rune-flavored, not byte-flavored" distinction.
int isprintrune(Rune);

static int minspan = MINSPAN;

static void
usage(void)
{
    fprint(2, "usage: %s [-m min] [file...]\n", argv0);
    exits("usage");
}

void
main(int argc, char **argv)
{
    int i;

    ARGBEGIN{
    case 'm':
        minspan = atoi(EARGF(usage()));
        break;
    default:
        usage();
        break;
    }ARGEND
    Binit(&fout, 1, OWRITE);
    if(argc < 1) {
        stringit("/fd/0");
        exits(0);
    }

    for(i = 0; i < argc; i++) {
        if(argc > 2)
            print("%s:\n", argv[i]);

        stringit(argv[i]);
    }

    exits(0);
}

void
stringit(char *str)
{
    long posn, start;
    int cnt = 0;
    long c;

    Rune buf[BUFSIZE];

    if ((fin = Bopen(str, OREAD)) == 0) {
        perror("open");
        return;
    }

    start = 0;
    posn = Boffset(fin);
    while((c = Bgetrune(fin)) >= 0) {
        if(isprintrune(c)) {
            if(start == 0)
                start = posn;
            buf[cnt++] = c;
            if(cnt == BUFSIZE-1) {
                buf[cnt] = 0;
                Bprint(&fout, "%8ld: %S ...\n", start, buf);
                start = 0;
                cnt = 0;
            }
        } else {
             if(cnt >= minspan) {
                buf[cnt] = 0;
                Bprint(&fout, "%8ld: %S\n", start, buf);
            }
            start = 0;
            cnt = 0;
        }   
        posn = Boffset(fin);
    }

    if(cnt >= minspan){
        buf[cnt] = 0;
        Bprint(&fout, "%8ld: %S\n", start, buf);
    }
    Bterm(fin);
}

int
isprintrune(Rune r)
{
    if (r != Runeerror)
    if ((r >= ' ' && r < 0x7F) || r > 0xA0)
        return 1;
    return 0;
}
/*e: misc/strings.c */
