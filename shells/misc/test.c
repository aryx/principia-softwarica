/*s: misc/test.c */
/*
 * POSIX standard
 *	test expression
 *	[ expression ]
 *
 * Plan 9 additions:
 *	-A file exists and is append-only
 *	-L file exists and is exclusive-use
 *	-T file exists and is temporary
 */

#include <u.h>
#include <libc.h>

/*s: function [[EQ]] */
#define EQ(a,b)	((tmp=a)==0?0:(strcmp(tmp,b)==0))
/*e: function [[EQ]] */

/*s: global [[ap]] */
int	ap;
/*e: global [[ap]] */
/*s: global [[ac]] */
int	ac;
/*e: global [[ac]] */
/*s: global [[av]] */
char	**av;
/*e: global [[av]] */
/*s: global [[tmp]] */
static char	*tmp;
/*e: global [[tmp]] */

void	synbad(char *, char *);
int	fsizep(char *);
int	isdir(char *);
int	isreg(char *);
int	isatty(int);
int	isint(char *, int *);
int	isolder(char *, char *);
int	isolderthan(char *, char *);
int	isnewerthan(char *, char *);
int	hasmode(char *, ulong);
int	tio(char *, int);
int	e(void), e1(void), e2(void), e3(void);
char	*nxtarg(int);

/*s: function main (misc/test.c) */
void
main(int argc, char *argv[])
{
    int r;
    char *c;

    ac = argc; av = argv; ap = 1;
    if(EQ(argv[0],"[")) {
        if(!EQ(argv[--ac],"]"))
            synbad("] missing","");
    }
    argv[ac] = 0;
    if (ac<=1)
        exits("usage");
    r = e();
    /*
     * nice idea but short-circuit -o and -a operators may have
     * not consumed their right-hand sides.
     */
    if(false && (c = nxtarg(1)) != nil)
        synbad("unexpected operator/operand: ", c);
    exits(r? nil : "false");
}
/*e: function main (misc/test.c) */

/*s: function [[nxtarg]] */
char *
nxtarg(int mt)
{
    if(ap>=ac){
        if(mt){
            ap++;
            return(0);
        }
        synbad("argument expected","");
    }
    return(av[ap++]);
}
/*e: function [[nxtarg]] */

/*s: function [[nxtintarg]] */
bool
nxtintarg(int *pans)
{
    if(ap<ac && isint(av[ap], pans)){
        ap++;
        return true;
    }
    return false;
}
/*e: function [[nxtintarg]] */

/*s: function [[e]] */
int
e(void)
{
    int p1;

    p1 = e1();
    if (EQ(nxtarg(1), "-o"))
        return(p1 || e());
    ap--;
    return(p1);
}
/*e: function [[e]] */

/*s: function [[e1]] */
int
e1(void)
{
    int p1;

    p1 = e2();
    if (EQ(nxtarg(1), "-a"))
        return (p1 && e1());
    ap--;
    return(p1);
}
/*e: function [[e1]] */

/*s: function [[e2]] */
int
e2(void)
{
    if (EQ(nxtarg(0), "!"))
        return(!e2());
    ap--;
    return(e3());
}
/*e: function [[e2]] */

/*s: function [[e3]] */
bool
e3(void)
{
    int p1, int1, int2;
    char *a, *p2;

    a = nxtarg(0);
    if(EQ(a, "(")) {
        p1 = e();
        if(!EQ(nxtarg(0), ")"))
            synbad(") expected","");
        return(p1);
    }

    if(EQ(a, "-A"))
        return(hasmode(nxtarg(0), DMAPPEND));

    if(EQ(a, "-L"))
        return(hasmode(nxtarg(0), DMEXCL));

    if(EQ(a, "-T"))
        return(hasmode(nxtarg(0), DMTMP));

    if(EQ(a, "-f"))
        return(isreg(nxtarg(0)));

    if(EQ(a, "-d"))
        return(isdir(nxtarg(0)));

    if(EQ(a, "-r"))
        return(tio(nxtarg(0), 4));

    if(EQ(a, "-w"))
        return(tio(nxtarg(0), 2));

    if(EQ(a, "-x"))
        return(tio(nxtarg(0), 1));

    if(EQ(a, "-e"))
        return(tio(nxtarg(0), 0));

    if(EQ(a, "-c"))
        return false;

    if(EQ(a, "-b"))
        return false;

    if(EQ(a, "-u"))
        return false;

    if(EQ(a, "-g"))
        return false;

    if(EQ(a, "-s"))
        return(fsizep(nxtarg(0)));

    if(EQ(a, "-t"))
        if(ap>=ac)
            return(isatty(1));
        else if(nxtintarg(&int1))
            return(isatty(int1));
        else
            synbad("not a valid file descriptor number ", "");

    if(EQ(a, "-n"))
        return(!EQ(nxtarg(0), ""));
    if(EQ(a, "-z"))
        return(EQ(nxtarg(0), ""));

    p2 = nxtarg(1);
    if (p2==0)
        return(!EQ(a,""));
    if(EQ(p2, "="))
        return(EQ(nxtarg(0), a));

    if(EQ(p2, "!="))
        return(!EQ(nxtarg(0), a));

    if(EQ(p2, "-older"))
        return(isolder(nxtarg(0), a));

    if(EQ(p2, "-ot"))
        return(isolderthan(nxtarg(0), a));

    if(EQ(p2, "-nt"))
        return(isnewerthan(nxtarg(0), a));

    if(!isint(a, &int1))
        synbad("unexpected operator/operand: ", p2);

    if(nxtintarg(&int2)){
        if(EQ(p2, "-eq"))
            return(int1==int2);
        if(EQ(p2, "-ne"))
            return(int1!=int2);
        if(EQ(p2, "-gt"))
            return(int1>int2);
        if(EQ(p2, "-lt"))
            return(int1<int2);
        if(EQ(p2, "-ge"))
            return(int1>=int2);
        if(EQ(p2, "-le"))
            return(int1<=int2);
    }

    synbad("unknown operator ",p2);
    return false;		/* to shut ken up */
}
/*e: function [[e3]] */

/*s: function [[tio]] */
bool
tio(char *a, int f)
{
    return access (a, f) >= 0;
}
/*e: function [[tio]] */

/*s: function [[hasmode]] */
/*
 * note that the name strings pointed to by Dir members are
 * allocated with the Dir itself (by the same call to malloc),
 * but are not included in sizeof(Dir), so copying a Dir won't
 * copy the strings it points to.
 */
bool
hasmode(char *f, ulong m)
{
    bool r;
    Dir *dir;

    dir = dirstat(f);
    if (dir == nil)
        return false;
    r = (dir->mode & m) != 0;
    free(dir);
    return r;
}
/*e: function [[hasmode]] */

/*s: function [[isdir]] */
bool
isdir(char *f)
{
    return hasmode(f, DMDIR);
}
/*e: function [[isdir]] */

/*s: function [[isreg]] */
bool
isreg(char *f)
{
    bool r;
    Dir *dir;

    dir = dirstat(f);
    if (dir == nil)
        return false;
    r = (dir->mode & DMDIR) == 0;
    free(dir);
    return r;
}
/*e: function [[isreg]] */

/*s: function [[isatty]] */
int
isatty(int fd)
{
    int r;
    Dir *d1, *d2;

    d1 = dirfstat(fd);
    d2 = dirstat("/dev/cons");
    if (d1 == nil || d2 == nil)
        r = 0;
    else
        r = d1->type == d2->type && d1->dev == d2->dev &&
            d1->qid.path == d2->qid.path;
    free(d1);
    free(d2);
    return r;
}
/*e: function [[isatty]] */

/*s: function [[fsizep]] */
bool
fsizep(char *f)
{
    bool r;
    Dir *dir;

    dir = dirstat(f);
    if (dir == nil)
        return false;
    r = dir->length > 0;
    free(dir);
    return r;
}
/*e: function [[fsizep]] */

/*s: function [[synbad]] */
void
synbad(char *s1, char *s2)
{
    int len;

    write(2, "test: ", 6);
    if ((len = strlen(s1)) != 0)
        write(2, s1, len);
    if ((len = strlen(s2)) != 0)
        write(2, s2, len);
    write(2, "\n", 1);
    exits("bad syntax");
}
/*e: function [[synbad]] */

/*s: function [[isint]] */
int
isint(char *s, int *pans)
{
    char *ep;

    *pans = strtol(s, &ep, 0);
    return (*ep == 0);
}
/*e: function [[isint]] */

/*s: function [[isolder]] */
bool
isolder(char *pin, char *f)
{
    int r, rel;
    ulong n, m;
    char *p = pin;
    Dir *dir;

    dir = dirstat(f);
    if (dir == nil)
        return false;

    /* parse time */
    n = 0;
    rel = 0;
    while(*p){
        m = strtoul(p, &p, 0);
        switch(*p){
        case 0:
            n = m;
            break;
        case 'y':
            m *= 12;
            /* fall through */
        case 'M':
            m *= 30;
            /* fall through */
        case 'd':
            m *= 24;
            /* fall through */
        case 'h':
            m *= 60;
            /* fall through */
        case 'm':
            m *= 60;
            /* fall through */
        case 's':
            n += m;
            p++;
            rel = 1;
            break;
        default:
            synbad("bad time syntax, ", pin);
        }
    }
    if (!rel)
        m = n;
    else{
        m = time(0);
        if (n > m)		/* before epoch? */
            m = 0;
        else
            m -= n;
    }
    r = dir->mtime < m;
    free(dir);
    return r;
}
/*e: function [[isolder]] */

/*s: function [[isolderthan]] */
int
isolderthan(char *a, char *b)
{
    int r;
    Dir *ad, *bd;

    ad = dirstat(a);
    bd = dirstat(b);
    if (ad == nil || bd == nil)
        r = 0;
    else
        r = ad->mtime > bd->mtime;
    free(ad);
    free(bd);
    return r;
}
/*e: function [[isolderthan]] */

/*s: function [[isnewerthan]] */
int
isnewerthan(char *a, char *b)
{
    int r;
    Dir *ad, *bd;

    ad = dirstat(a);
    bd = dirstat(b);
    if (ad == nil || bd == nil)
        r = 0;
    else
        r = ad->mtime < bd->mtime;
    free(ad);
    free(bd);
    return r;
}
/*e: function [[isnewerthan]] */
/*e: misc/test.c */
