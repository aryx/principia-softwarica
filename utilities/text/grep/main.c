/*s: grep/main.c */
#include    "grep.h"

/*s: constant [[validflags]](grep) */
char *validflags = "bchiLlnsv";
/*e: constant [[validflags]](grep) */
/*s: function [[usage]](grep) */
void
usage(void)
{
    fprint(STDERR, "usage: grep [-%s] [-e pattern] [-f patternfile] [file ...]\n", validflags);
    exits("usage");
}
/*e: function [[usage]](grep) */

/*s: function [[main]](grep) */
void
main(int argc, char *argv[])
{
    int i;
    int status;

    ARGBEGIN {
    default:
        if(utfrune(validflags, ARGC()) == nil)
            usage();
        flags[ARGC()]++;
        break;

    case 'e':
        flags['e']++;
        lineno = 0;
        str2top(EARGF(usage()));
        break;

    case 'f':
        flags['f']++;
        filename = EARGF(usage());
        rein = Bopen(filename, OREAD);
        if(rein == nil) {
            fprint(STDERR, "grep: can't open %s: %r\n", filename);
            exits("open");
        }
        lineno = 1;
        str2top(filename);
        break;
    } ARGEND

    if(flags['f'] == 0 && flags['e'] == 0) {
        if(argc <= 0)
            usage();
        str2top(argv[0]);
        argc--;
        argv++;
    }

    follow = mal(maxfollow*sizeof(*follow));
    state0 = initstate_(topre.beg);

    Binit(&bout, STDOUT, OWRITE);
    switch(argc) {
    case 0:
        status = search(nil, 0);
        break;
    case 1:
        status = search(argv[0], 0);
        break;
    default:
        status = 0;
        for(i=0; i<argc; i++)
            status |= search(argv[i], Hflag);
        break;
    }
    if(status)
        exits(nil);
    exits("no matches");
}
/*e: function [[main]](grep) */

/*s: function [[search]](grep) */
int
search(char *file, int flag)
{
    State *s, *ns;
    fdt fid;
    int c, eof, nl, empty;
    long count, lineno, n;
    uchar *elp, *lp, *bol;

    if(file == nil) {
        file = "stdin";
        fid = STDIN;
        flag |= Bflag;
    } else
        fid = open(file, OREAD);

    if(fid < 0) {
        fprint(STDERR, "grep: can't open %s: %r\n", file);
        return 0;
    }

    if(flags['b'])
        flag ^= Bflag;      /* dont buffer output */
    if(flags['c'])
        flag |= Cflag;      /* count */
    if(flags['h'])
        flag &= ~Hflag;     /* do not print file name in output */
    if(flags['i'])
        flag |= Iflag;      /* fold upper-lower */
    if(flags['l'])
        flag |= Llflag;     /* print only name of file if any match */
    if(flags['L'])
        flag |= LLflag;     /* print only name of file if any non match */
    if(flags['n'])
        flag |= Nflag;      /* count only */
    if(flags['s'])
        flag |= Sflag;      /* status only */
    if(flags['v'])
        flag |= Vflag;      /* inverse match */

    s = state0;
    lineno = 0;
    count = 0;
    eof = 0;
    empty = 1;
    nl = 0;
    lp = u.buf;
    bol = lp;

loop0:
    n = lp-bol;
    if(n > sizeof(u.pre))
        n = sizeof(u.pre);
    memmove(u.buf-n, bol, n);
    bol = u.buf-n;
    n = read(fid, u.buf, sizeof(u.buf));
    /* if file has no final newline, simulate one to emit matches to last line */
    if(n > 0) {
        empty = 0;
        nl = u.buf[n-1]=='\n';
    } else {
        if(n < 0){
            fprint(STDERR, "grep: read error on %s: %r\n", file);
            return count != 0;
        }
        if(!eof && !nl && !empty) {
            u.buf[0] = '\n';
            n = 1;
            eof = 1;
        }
    }
    if(n <= 0) {
        close(fid);
        if(flag & Cflag) {
            if(flag & Hflag)
                Bprint(&bout, "%s:", file);
            Bprint(&bout, "%ld\n", count);
        }
        if(((flag&Llflag) && count != 0) || ((flag&LLflag) && count == 0))
            Bprint(&bout, "%s\n", file);
        Bflush(&bout);
        return count != 0;
    }
    lp = u.buf;
    elp = lp+n;
    if(flag & Iflag)
        goto loopi;

/*
 * normal character loop
 */
loop:
    c = *lp;
    ns = s->next[c];
    if(ns == 0) {
        increment(s, c);
        goto loop;
    }
//  if(flags['2'])
//      if(s->match)
//          print("%d: %.2x**\n", s, c);
//      else
//          print("%d: %.2x\n", s, c);
    lp++;
    s = ns;
    if(c == '\n') {
        lineno++;
        if(!!s->match == !(flag&Vflag)) {
            count++;
            if(flag & (Cflag|Sflag|Llflag|LLflag))
                goto cont;
            if(flag & Hflag)
                Bprint(&bout, "%s:", file);
            if(flag & Nflag)
                Bprint(&bout, "%ld: ", lineno);
            /* suppress extra newline at EOF unless we are labeling matches with file name */
            Bwrite(&bout, bol, lp-bol-(eof && !(flag&Hflag)));
            if(flag & Bflag)
                Bflush(&bout);
        }
        if((lineno & Flshcnt) == 0)
            Bflush(&bout);
    cont:
        bol = lp;
    }
    if(lp != elp)
        goto loop;
    goto loop0;

/*
 * character loop for -i flag
 * for speed
 */
loopi:
    c = *lp;
    if(c >= 'A' && c <= 'Z')
        c += 'a'-'A';
    ns = s->next[c];
    if(ns == 0) {
        increment(s, c);
        goto loopi;
    }
    lp++;
    s = ns;
    if(c == '\n') {
        lineno++;
        if(!!s->match == !(flag&Vflag)) {
            count++;
            if(flag & (Cflag|Sflag|Llflag|LLflag))
                goto conti;
            if(flag & Hflag)
                Bprint(&bout, "%s:", file);
            if(flag & Nflag)
                Bprint(&bout, "%ld: ", lineno);
            /* suppress extra newline at EOF unless we are labeling matches with file name */
            Bwrite(&bout, bol, lp-bol-(eof && !(flag&Hflag)));
            if(flag & Bflag)
                Bflush(&bout);
        }
        if((lineno & Flshcnt) == 0)
            Bflush(&bout);
    conti:
        bol = lp;
    }
    if(lp != elp)
        goto loopi;
    goto loop0;
}
/*e: function [[search]](grep) */
/*s: function [[initstate]](grep) */
State*
initstate_(Re *r)
{
    State *s;
    int i;

    addcase(r);
    if(flags['1'])
        reprint("r", r);
    nfollow = 0;
    gen++;
    fol1(r, Cbegin);
    follow[nfollow++] = r;
    qsort(follow, nfollow, sizeof(*follow), fcmp);

    s = sal(nfollow);
    for(i=0; i<nfollow; i++)
        s->re[i] = follow[i];
    return s;
}
/*e: function [[initstate]](grep) */
/*e: grep/main.c */
