/*s: byte/split.c */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */
#include <bio.h>
#include <regexp.h>

/*s: globals split.c */
char    *stem = "x";
/*x: globals split.c */
char    *suffix = "";
/*x: globals split.c */
char    digit[] = "0123456789";
char    suff[] = "aa";
char    name[200];
/*x: globals split.c */
Biobuf  bout;
Biobuf  *output = &bout;
/*x: globals split.c */
char *pattern = nil;
/*e: globals split.c */

// forward decls
extern int nextfile(void);
extern int matchfile(Resub*);
extern void openf(void);
extern char *fold(char*,int);
extern void usage(void);
extern void badexp(void);

/*s: function [[main]](split.c) */
void
main(int argc, char *argv[])
{
    /*s: [[main]](split.c) locals */
    int n = 1000;
    /*x: [[main]](split.c) locals */
    // ??
    bool xflag = false;
    // ??
    bool iflag = false;
    /*x: [[main]](split.c) locals */
    Biobuf bin;
    Biobuf *b = &bin;
    /*x: [[main]](split.c) locals */
    char *line;
    /*x: [[main]](split.c) locals */
    char buf[256];
    /*x: [[main]](split.c) locals */
    Reprog *exp;
    /*e: [[main]](split.c) locals */

    ARGBEGIN {
    /*s: [[main]](split.c) switch flag character cases */
    case 'l':
    case 'n':
        n=atoi(EARGF(usage()));
        break;
    case 'e':
        pattern = strdup(EARGF(usage()));
        break;
    case 'f':
        stem = strdup(EARGF(usage()));
        break;
    case 's':
        suffix = strdup(EARGF(usage()));
        break;
    case 'x':
        xflag = true;
        break;
    case 'i':
        iflag = true;
        break;
    /*e: [[main]](split.c) switch flag character cases */
    default:
        usage();
        break;

    } ARGEND;

    if(argc < 0 || argc > 1)
        usage();

    if(argc != 0) {
        b = Bopen(argv[0], OREAD);
        if(b == nil) {
            fprint(STDERR, "split: can't open %s: %r\n", argv[0]);
            exits("open");
        }
    } else
        Binit(b, STDIN, OREAD);

    /*s: [[main()]](split.c) if [[pattern]] */
    if(pattern) {
        Resub match[2];

        if(!(exp = regcomp(iflag? fold(pattern, strlen(pattern)):
            pattern)))
            badexp();
        memset(match, 0, sizeof match);
        matchfile(match);
        while((line=Brdline(b,'\n')) != 0) {
            memset(match, 0, sizeof match);
            line[Blinelen(b)-1] = 0;
            if(regexec(exp, iflag? fold(line, Blinelen(b)-1): line,
                match, 2)) {
                if(matchfile(match) && xflag)
                    continue;
            } else if(output == 0)
                nextfile(); /* at most once */
            Bwrite(output, line, Blinelen(b)-1);
            Bputc(output, '\n');
        }
    }
    /*e: [[main()]](split.c) if [[pattern]] */
    else {
        int linecnt = n;

        while((line=Brdline(b,'\n')) != 0) {
            if(++linecnt > n) {
                nextfile();
                linecnt = 1;
            }
            Bwrite(output, line, Blinelen(b));
        }

        /*
         * in case we didn't end with a newline, tack whatever's 
         * left onto the last file
         */
        while((n = Bread(b, buf, sizeof(buf))) > 0)
            Bwrite(output, buf, n);
    }
    if(b != nil)
        Bterm(b);
    exits(nil);
}
/*e: function [[main]](split.c) */

/*s: function [[nextfile]](split.c) */
bool
nextfile(void)
{
    static bool canopen = true;

    if(suff[0] > 'z') {
        if(canopen)
            fprint(STDERR, "split: file %szz not split\n",stem);
        canopen = false;
    } else {
        snprint(name, sizeof name, "%s%s", stem, suff);
        if(++suff[1] > 'z') 
            suff[1] = 'a', ++suff[0];
        openf();
    }
    return canopen;
}
/*e: function [[nextfile]](split.c) */
/*s: function [[matchfile]](split.c) */
int
matchfile(Resub *match)
{
    if(match[1].s.sp) {
        int len = match[1].e.ep - match[1].s.sp;

        strncpy(name, match[1].s.sp, len);
        strcpy(name+len, suffix);
        openf();
        return 1;
    } 
    return nextfile();
}
/*e: function [[matchfile]](split.c) */
/*s: function [[openf]](split.c) */
void
openf(void)
{
    static fdt fd = 0;

    Bflush(output);
    Bterm(output);
    if(fd > 0)
        close(fd);
    fd = create(name,OWRITE,0666);
    if(fd < 0) {
        fprint(STDERR, "grep: can't create %s: %r\n", name);
        exits("create");
    }
    Binit(output, fd, OWRITE);
}
/*e: function [[openf]](split.c) */
/*s: function [[fold]](split.c) */
char *
fold(char *s, int n)
{
    static char *fline;
    static int linesize = 0;
    char *t;

    if(linesize < n+1){
        fline = realloc(fline,n+1);
        linesize = n+1;
    }
    for(t=fline; *t++ = tolower(*s++); )
        continue;
        /* we assume the 'A'-'Z' only appear as themselves
         * in a utf encoding.
         */
    return fline;
}
/*e: function [[fold]](split.c) */
/*s: function [[usage]](split.c) */
void
usage(void)
{
    fprint(STDERR, "usage: split [-n num] [-e exp] [-f stem] [-s suff] [-x] [-i] [file]\n");
    exits("usage");
}
/*e: function [[usage]](split.c) */
/*s: function [[badexp]](split.c) */
void
badexp(void)
{
    fprint(STDERR, "split: bad regular expression\n");
    exits("bad regular expression");
}
/*e: function [[badexp]](split.c) */
/*e: byte/split.c */
