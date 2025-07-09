/*s: files/cp.c */
/*s: plan9 includes */
#include <u.h>
#include <libc.h>
/*e: plan9 includes */

/*s: constant [[DEFB]](cp.c) */
#define DEFB    (8*1024)
/*e: constant [[DEFB]](cp.c) */

/*s: global [[failed]](cp.c) */
bool failed;
/*e: global [[failed]](cp.c) */
/*s: global flags(cp.c) */
// keep gid
bool gflag = false;
// keep uid (-u implies -g)
bool uflag = false;
// keep time and mode
bool xflag = false;
/*e: global flags(cp.c) */

void copy(char *from, char *to, bool todir);
errorneg1 copy1(fdt fdf, fdt fdt, char *from, char *to);

/*s: function [[main]](cp.c) */
void
main(int argc, char *argv[])
{
    Dir *dirb;
    int i;
    bool todir = false;

    ARGBEGIN {
    case 'g':
        gflag = true;
        break;
    case 'u':
        uflag = true;
        gflag = true;
        break;
    case 'x':
        xflag = true;
        break;
    default:
        goto usage;
    } ARGEND

    if(argc < 2)
        goto usage;

    dirb = dirstat(argv[argc-1]);
    if(dirb!=nil && (dirb->mode&DMDIR))
        todir=true;
    if(argc>2 && !todir){
        fprint(STDERR, "cp: %s not a directory\n", argv[argc-1]);
        exits("bad usage");
    }

    for(i=0; i<argc-1; i++)
        copy(argv[i], argv[argc-1], todir);

    if(failed)
        exits("errors");
    exits(nil);
/*s: label [[usage]] in [[main]](cp.c) */
usage:
    fprint(STDERR, "usage:\tcp [-gux] fromfile tofile\n");
    fprint(STDERR, "\tcp [-x] fromfile ... todir\n");
    exits("usage");
/*e: label [[usage]] in [[main]](cp.c) */
}
/*e: function [[main]](cp.c) */
/*s: function [[samefile]](cp.c) */
bool
samefile(Dir *a, char *an, char *bn)
{
    Dir *b;
    bool ret = false;

    b=dirstat(bn);
    if(b != nil)
    if(b->qid.type==a->qid.type)
    if(b->qid.path==a->qid.path)
    if(b->qid.vers==a->qid.vers)
    if(b->dev==a->dev)
    if(b->type==a->type){
        fprint(STDERR, "cp: %s and %s are the same file\n", an, bn);
        ret = true;
    }
    free(b);
    return ret;
}
/*e: function [[samefile]](cp.c) */
/*s: function [[copy]] */
void
copy(char *from, char *to, bool todir)
{
    Dir *dirb, dirt;
    char name[256];
    // fd from, fd to
    fdt fdf, fdt;
    int mode;

    if(todir){
        char *s, *elem;
        elem=s=from;
        while(*s++)
            if(s[-1]=='/')
                elem=s;
        sprint(name, "%s/%s", to, elem);
        to=name;
    }

    if((dirb=dirstat(from))==nil){
        fprint(STDERR,"cp: can't stat %s: %r\n", from);
        failed = true;
        return;
    }
    mode = dirb->mode;
    if(mode&DMDIR){
        fprint(STDERR, "cp: %s is a directory\n", from);
        free(dirb);
        failed = true;
        return;
    }
    if(samefile(dirb, from, to)){
        free(dirb);
        failed = true;
        return;
    }
    mode &= 0777;
    fdf=open(from, OREAD);
    if(fdf<0){
        fprint(STDERR, "cp: can't open %s: %r\n", from);
        free(dirb);
        failed = true;
        return;
    }
    fdt=create(to, OWRITE, mode);
    if(fdt<0){
        fprint(STDERR, "cp: can't create %s: %r\n", to);
        close(fdf);
        free(dirb);
        failed = true;
        return;
    }
    if(copy1(fdf, fdt, from, to)==OK_0 && (xflag || gflag || uflag)){
        nulldir(&dirt);
        if(xflag){
            dirt.mtime = dirb->mtime;
            dirt.mode = dirb->mode;
        }
        if(uflag)
            dirt.uid = dirb->uid;
        if(gflag)
            dirt.gid = dirb->gid;
        if(dirfwstat(fdt, &dirt) < 0)
            fprint(STDERR, "cp: warning: can't wstat %s: %r\n", to);
    }           
    free(dirb);
    close(fdf);
    close(fdt);
}
/*e: function [[copy]] */
/*s: function [[copy1]] */
errorneg1
copy1(fdt fdf, fdt fdt, char *from, char *to)
{
    char *buf;
    long n, n1, rcount;
    errorneg1 rv = OK_0;
    char err[ERRMAX];

    buf = malloc(DEFB);
    /* clear any residual error */
    err[0] = '\0';
    errstr(err, ERRMAX);

    // copy (read and write) as long as n read <= 0
    for(rcount=0;; rcount++) {
        n = read(fdf, buf, DEFB);
        if(n <= 0)
            break;
        n1 = write(fdt, buf, n);
        if(n1 != n) {
            fprint(STDERR, "cp: error writing %s: %r\n", to);
            failed = true;
            rv = ERROR_NEG1;
            break;
        }
    }
    if(n < 0) {
        fprint(2, "cp: error reading %s: %r\n", from);
        failed = true;
        rv = ERROR_NEG1;
    }
    free(buf);
    return rv;
}
/*e: function [[copy1]] */
/*e: files/cp.c */
