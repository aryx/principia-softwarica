/*s: git9/walk.c */
#include <u.h>
#include <libc.h>
#include "git.h"

typedef struct Idxed    Idxed;
typedef struct Idxent   Idxent;

/*s: constant [[NCACHE]] */
#define NCACHE 4096
/*e: constant [[NCACHE]] */

/*s: enum WalkFlags */
// do not reorder, some code assume Tflg is the last
enum WalkFlags {
    Rflg    = 1 << 0,
    Mflg    = 1 << 1,
    Aflg    = 1 << 2,
    Uflg    = 1 << 3,
    /* everything after this is not an error */
    Tflg    = 1 << 4,
};
/*e: enum WalkFlags */

struct Idxed {
    char**  cache;
    int n;
    int max;
};

Idxed   idxtab[NCACHE];

char    repopath[1024];
/*s: global [[wdirpath]](walk.c) */
char    wdirpath[1024];
/*e: global [[wdirpath]](walk.c) */
char    relapath[1024];

char    *rstr   = "R ";
char    *mstr   = "M ";
char    *astr   = "A ";
char    *ustr   = "U ";
char    *tstr   = "T ";

char    *bdir = ".git/fs/HEAD/tree";

int nslash;
int nrel;
/*s: global [[quiet]](walk.c) */
bool quiet;
/*e: global [[quiet]](walk.c) */
int dirty;
/*s: global [[isindexed]](walk.c) */
bool isindexed = true;
/*e: global [[isindexed]](walk.c) */
/*s: global [[intree]](walk.c) */
bool intree;
/*e: global [[intree]](walk.c) */
/*s: global [[printflg]](walk.c) */
// bitset<enum<WalkFlags>>
int printflg;
/*e: global [[printflg]](walk.c) */

/*s: global [[idx]](walk.c) */
// growing_array<Idxent> (used = nidx, allocated = idxsz)
Idxent  *idx;
/*e: global [[idx]](walk.c) */
/*s: global [[idxsz]](walk.c) */
int idxsz;
/*e: global [[idxsz]](walk.c) */
/*s: global [[nidx]](walk.c) */
int nidx;
/*e: global [[nidx]](walk.c) */

/*s: global [[cleanidx]] */
bool cleanidx = false;   /* skip tree check for checkedin() */
/*e: global [[cleanidx]] */
/*s: global [[staleidx]](walk.c) */
bool staleidx = false;
/*e: global [[staleidx]](walk.c) */

/*s: global [[wdir]](walk.c) */
// growing_array<Idxent> (used = nwdir, allocated = wdirsz)
Idxent  *wdir;
/*e: global [[wdir]](walk.c) */
/*s: global [[wdirsz]](walk.c) */
int wdirsz;
/*e: global [[wdirsz]](walk.c) */
/*s: global [[nwdir]](walk.c) */
int nwdir;
/*e: global [[nwdir]](walk.c) */


void    loadwdir(char*);

/*s: function [[checkedin]] */
bool
checkedin(Idxent *e, bool change)
{
    char *p;
    int r;

    /* clean index, no need to check tree */
    if(cleanidx)
        return e >= &idx[0] && e < &idx[nidx] && e->state != 'R';

    if((p = smprint("%s/%s", bdir, e->path)) == nil)
        sysfatal("smprint: %r");

    r = access(p, AEXIST);
    if(r == 0 && change){
        if(e->state != 'R')
            e->state = 'T';
        staleidx = true;
    }
    free(p);

    return r == 0;
}
/*e: function [[checkedin]] */

/*s: function [[pathcmp]] */
/*
 * Tricky; we want to know if a file or dir is indexed,
 * but a dir is only indexed if we have a file with dir/
 * listed in the index.
 *
 * as a result, we need to add a virtual '/' at the end
 * of the path if we're doing it, so if we have:
 *  foo.bar/x
 *  foo/y
 * and we want to find out if foo is a directory we should
 * descend into, we need to compare as though foo/ ended
 * with a '/', or we'll bsearch down do foo.bar, not foo.
 *
 * this code resembles entcmp() in util.c, but differs
 * because we're comparing whole paths.
 */
int
pathcmp(char *sa, char *sb, int sadir)
{
    unsigned a, b;

    while(1){
        a = *sa++;
        b = *sb++;
        if(a != b){
            if(a == 0 && sadir)
                a = '/';
            if(a == '/' && b == '/')
                return 0;
            return (a > b) ? 1 : -1;
        }
        if(a == 0)
            return 0;
    }
}
/*e: function [[pathcmp]] */

/*s: function [[indexed]] */
int
indexed(char *path, int dir)
{
    int lo, hi, mid, r;

    r = -1;
    lo = 0;
    hi = nidx-1;
    while(lo <= hi){
        mid = (hi + lo) / 2;
        r = pathcmp(path, idx[mid].path, dir);
        if(r < 0)
            hi = mid-1;
        else if(r > 0)
            lo = mid+1;
        else
            break;
    }
    return r == 0;
}
/*e: function [[indexed]] */

/*s: function [[idxcmp (git9/walk.c)]] */
int
idxcmp(void *pa, void *pb)
{
    Idxent *a, *b;
    int c;

    a = (Idxent*)pa;
    b = (Idxent*)pb;
    if((c = strcmp(a->path, b->path)) != 0)
        return c;
    /* maintain load order if name is identical */
    return a->order < b->order ? -1 : 1;
}
/*e: function [[idxcmp (git9/walk.c)]] */

/*s: function [[samedata]] */
/*
 * compares whether the indexed entry 'a'
 * has the same contents and mode as
 * the entry on disk 'b'; if the indexed
 * entry is nil, does a deep comparison
 * of the checked out file and the file
 * checked in.
 */
int
samedata(Idxent *a, Idxent *b)
{
    char *gitpath, ba[IOUNIT], bb[IOUNIT];
    int fa, fb, na, nb, same;
    Dir *da, *db;

    if(a != nil){
        if(a->qid.path == b->qid.path
        && a->qid.vers == b->qid.vers
        && a->qid.type == b->qid.type
        && a->mode == b->mode
        && a->mode != 0)
            return 1;
    }

    same = 0;
    da = nil;
    db = nil;
    if((gitpath = smprint("%s/%s", bdir, b->path)) == nil)
        sysfatal("smprint: %r");
    fa = open(gitpath, OREAD);
    fb = open(b->path, OREAD);
    if(fa == -1 || fb == -1)
        goto mismatch;
    da = dirfstat(fa);
    db = dirfstat(fb);
    if(da == nil || db == nil)
        goto mismatch;
    if((da->mode&0100) != (db->mode&0100))
        goto mismatch;
    if(da->length != db->length)
        goto mismatch;
    while(1){
        if((na = readn(fa, ba, sizeof(ba))) == -1)
            goto mismatch;
        if((nb = readn(fb, bb, sizeof(bb))) == -1)
            goto mismatch;
        if(na != nb)
            goto mismatch;
        if(na == 0)
            break;
        if(memcmp(ba, bb, na) != 0)
            goto mismatch;
    }
    if(a != nil){
        a->qid = db->qid;
        a->mode = db->mode;
        staleidx = 1;
    }
    same = 1;

mismatch:
    free(da);
    free(db);
    if(fa != -1)
        close(fa);
    if(fb != -1)
        close(fb);
    return same;
}
/*e: function [[samedata]] */

/*s: function [[loadent]] */
void
loadent(char *dir, Dir *d, bool fullpath)
{
    char *path;
    Idxent *e;

    if(fullpath)
        path = estrdup(dir);
    else if((path = smprint("%s/%s", dir, d->name)) == nil)
        sysfatal("smprint: %r");

    cleanname(path);
    if(!intree && (printflg & Uflg) == 0 && !indexed(path, d->qid.type & QTDIR)){
        free(path);
        return;
    }
    if(d->qid.type & QTDIR){
        loadwdir(path);
        free(path);
    }else{
        if(nwdir == wdirsz){
            wdirsz += wdirsz/2;
            wdir = erealloc(wdir, wdirsz*sizeof(Idxent));
        }
        e = wdir + nwdir;
        e->path = path;
        e->qid = !intree? d->qid : (Qid){-1,-1,-1};
        e->mode = d->mode;
        e->order = nwdir;
        e->state = 'T';
        nwdir++;
    }
}
/*e: function [[loadent]] */
/*s: function [[loadwdir]] */
void
loadwdir(char *path)
{
    fdt fd;
    int i, n;
    Dir *d, *e;

    d = nil;
    e = nil;
    cleanname(path);
    if(!intree
    && strncmp(path, ".git", 4) == 0
    && (path[4] == '/' || path[4] == 0))
        return;
    // else

    if((fd = open(path, OREAD)) < 0)
        goto error;
    if((e = dirfstat(fd)) == nil)
        fprint(STDERR, "fstat: %r");
    if(e->qid.type & QTDIR)
        while((n = dirread(fd, &d)) > 0){
            for(i = 0; i < n; i++)
                loadent(path, &d[i], false);
            free(d);
        }
    else
        loadent(path, e, true);
error:
    free(e);
    if(fd != -1)
        close(fd);
}
/*e: function [[loadwdir]] */

/*s: function [[pfxmatch]] */
int
pfxmatch(char *p, char **pfx, int *pfxlen, int npfx)
{
    int i;

    if(p == nil)
        return 0;
    if(npfx == 0)
        return 1;
    for(i = 0; i < npfx; i++){
        if(strncmp(p, pfx[i], pfxlen[i]) != 0)
            continue;
        if(p[pfxlen[i]] == '/' || p[pfxlen[i]] == 0)
            return 1;
        if(strcmp(pfx[i], ".") == 0 || *pfx[i] == 0)
            return 1;
    }
    return 0;
}
/*e: function [[pfxmatch]] */

/*s: function [[reporel]] */
char*
reporel(char *s)
{
    char *p;
    int n;

    if(*s == '/')
        s = estrdup(s);
    else if((s = smprint("%s/%s", wdirpath, s)) == nil)
        sysfatal("smprint: %r");

    p = cleanname(s);
    n = strlen(repopath);
    if(strncmp(s, repopath, n) != 0)
        sysfatal("path outside repo: %s", s);
    p += n;
    if(*p == '/')
        p++;
    memmove(s, p, strlen(p)+1);
    return s;
}
/*e: function [[reporel]] */

/*s: function [[show (git9/walk.c)]] */
void
show(Biobuf *o, int flg, char *str, char *path)
{
    char *pa, *pb;
    int n;

    dirty |= flg;
    if(!quiet && (printflg & flg)){
        Bprint(o, str);
        n = nslash;
        if(n){
            for(pa = relapath, pb = path; *pa && *pb; pa++, pb++){
                if(*pa != *pb)
                    break;
                if(*pa == '/'){
                    n--;
                    path = pb+1;
                }
            }
            while(n-- > 0)
                Bprint(o, "../");
        }
        Bprint(o, "%s\n", path);
    }
}
/*e: function [[show (git9/walk.c)]] */

/*s: function [[findslashes]] */
void
findslashes(char *path)
{
    char *p;

    p = cleanname(path);
    if(p[0] == '.'){
        if(p[1] == '\0')
            return;
        else if(p[1] == '.' && (p[2] == '/' || p[2] == '\0'))
            sysfatal("relative path escapes git root");
    }
    
    snprint(relapath, sizeof relapath, "%s/", p);
    p = relapath;
    if(*p == '/')
        p++;

    for(; *p; p++)
        if(*p == '/')
            nslash++;
}
/*e: function [[findslashes]] */

/*s: function [[usage (git9/walk.c)]] */
void
usage(void)
{
    fprint(STDERR, "usage: %s [-qbcI] [-f filt] [-b base] [paths...]\n", argv0);
    exits("usage");
}
/*e: function [[usage (git9/walk.c)]] */
/*s: function [[main (git9/walk.c)]] */
void
main(int argc, char **argv)
{
    Biobuf *f; // .git/INDEX9
    Biobuf *o; // STDOUT
    /*s: [[main()]](walk.c) other locals */
    char *ln;
    int line;
    char *parts[4];
    /*x: [[main()]](walk.c) other locals */
    char **argrel;
    int *argn;
    /*x: [[main()]](walk.c) other locals */
    int i, j;
    int c;
    /*x: [[main()]](walk.c) other locals */
    char *p, *e, *base, xbuf[8];
    Hash h;
    Dir rn;
    /*x: [[main()]](walk.c) other locals */
    fdt wfd;
    Biobuf *w;
    /*e: [[main()]](walk.c) other locals */

    gitinit(repopath, sizeof(repopath), &nrel);
    if(getwd(wdirpath, sizeof(wdirpath)) == nil)
        sysfatal("getwd: %r");
    // !! chdir !!
    if(chdir(repopath) == ERROR_NEG1)
        sysfatal("chdir: %r");
    if(access(".git/fs/ctl", AEXIST) != 0)
        sysfatal("no running git/fs");

    ARGBEGIN{
    /*s: [[main()]](walk.c) command line processing */
    case 'q':
        quiet=true;
        break;
    /*x: [[main()]](walk.c) command line processing */
    case 'f':
        for(p = EARGF(usage()); *p; p++)
            switch(*p){
            case 'T':   printflg |= Tflg;   break;
            case 'A':   printflg |= Aflg;   break;
            case 'M':   printflg |= Mflg;   break;
            case 'R':   printflg |= Rflg;   break;
            case 'U':   printflg |= Uflg;   break;
            default:    usage();        break;
        }
        break;
    /*x: [[main()]](walk.c) command line processing */
    case 'b':
        isindexed = false;
        base = EARGF(usage());
        if(resolveref(&h, base) == ERROR_NEG1)
            sysfatal("no such ref '%s'", base);
        bdir = smprint(".git/fs/object/%H/tree", h);
        break;
    /*x: [[main()]](walk.c) command line processing */
    case 'I':
        /* invalidate index */
        staleidx = true;
        break;
    /*x: [[main()]](walk.c) command line processing */
    case 'r':
        findslashes(EARGF(usage()));
        break;
    /*x: [[main()]](walk.c) command line processing */
    case 'c':
        rstr = "";
        tstr = "";
        mstr = "";
        astr = "";
        ustr = "";
        break;
    /*e: [[main()]](walk.c) command line processing */
    default:
        usage();
    }ARGEND;

    if(printflg == 0)
        printflg = Tflg | Aflg | Mflg | Rflg;

    /*s: [[main()]](walk.c) initializations part 1 */
    argrel = emalloc(argc*sizeof(char*));
    argn = emalloc(argc*sizeof(int));
    for(i = 0; i < argc; i++){
        argrel[i] = reporel(argv[i]);
        argn[i] = strlen(argrel[i]);
    }
    /*e: [[main()]](walk.c) initializations part 1 */

    if(isindexed && !staleidx){
        f = Bopen(".git/INDEX9", OREAD);
        if(f == nil){
            staleidx = true;
            goto Stale;
        }
        /*s: [[main()]](walk.c) initializations part 2, when has [[.git/INDEX9]] */
        nidx = 0;
        idxsz = 32;
        idx = emalloc(idxsz*sizeof(Idxent)); // why not eamalloc?

        line = 0;
        while((ln = Brdstr(f, '\n', 1)) != nil){
            line++;
            /* allow blank lines */
            if(ln[0] == '\0' || ln[0] == '\n')
                continue;
            if(getfields(ln, parts, nelem(parts), 0, " \t") != nelem(parts))
                sysfatal(".git/INDEX9:%d: corrupt index", line);
            // else
            cleanname(parts[3]);
            /*s: [[main()]](walk.c) realloc [[idx]] if needed */
            if(nidx == idxsz){
                idxsz += idxsz/2;
                idx = erealloc(idx, idxsz*sizeof(Idxent));
            }
            /*e: [[main()]](walk.c) realloc [[idx]] if needed */

            idx[nidx].state = *parts[0];
            idx[nidx].qid = parseqid(parts[1]);
            idx[nidx].mode = strtol(parts[2], nil, 8);
            idx[nidx].path = estrdup(parts[3]);
            idx[nidx].order = nidx;
            nidx++;
            free(ln);
        }
        Bterm(f);
        /*e: [[main()]](walk.c) initializations part 2, when has [[.git/INDEX9]] */
    }
    else {
Stale:
        /*s: [[main()]](walk.c) initializations part 2, when stale */
        nwdir = 0;
        wdirsz = 32;
        wdir = emalloc(wdirsz*sizeof(Idxent));

        if(chdir(bdir) == -1)
            sysfatal("chdir: %r");

        /* load whole tree into index when stale */
        intree = 1;
        if(staleidx || argc == 0)
            loadwdir(".");
        else for(i = 0; i < argc; i++)
            loadwdir(argrel[i]);

        if(chdir(repopath) == -1)
            sysfatal("chdir: %r");

        /* use as index */
        idx = wdir;
        nidx = nwdir;
        idxsz = wdirsz;

        cleanidx = true;
        /*e: [[main()]](walk.c) initializations part 2, when stale */
    }
    /*s: [[main()]](walk.c) initializations part 3 */
    qsort(idx, nidx, sizeof(Idxent), idxcmp);

    nwdir = 0;
    wdirsz = 32;
    wdir = emalloc(wdirsz*sizeof(Idxent));

    intree = false;
    if(argc == 0)
        loadwdir(".");
    else for(i = 0; i < argc; i++)
        loadwdir(argrel[i]);
    qsort(wdir, nwdir, sizeof(Idxent), idxcmp);
    /*e: [[main()]](walk.c) initializations part 3 */
    o = Bfdopen(STDOUT, OWRITE);
    /*s: [[main()]](walk.c) sanity check [[o]] */
    if(o == nil)
        sysfatal("open out: %r");
    /*e: [[main()]](walk.c) sanity check [[o]] */

    /*s: [[main()]](walk.c) walking [[idx]] and [[wdir]] */
    i = 0;
    j = 0;
    while(i < nidx || j < nwdir){
        /* find the last entry we tracked for a path */
        while(i+1 < nidx && strcmp(idx[i].path, idx[i+1].path) == 0){
            staleidx = true;
            i++;
        }
        while(j+1 < nwdir && strcmp(wdir[j].path, wdir[j+1].path) == 0)
            j++;

        if(i < nidx && !pfxmatch(idx[i].path, argrel, argn, argc)){
            i++;
            continue;
        }
        if(i >= nidx)
            c = 1;
        else if(j >= nwdir)
            c = -1;
        else
            c = strcmp(idx[i].path, wdir[j].path);

        /* exists in both index and on disk */
        if(c == 0){
            /*s: [[main()]](walk.c) when walking and exists in both index and disk */
            if(idx[i].state == 'R'){
                if(checkedin(&idx[i], false))
                    show(o, Rflg, rstr, idx[i].path);
                else{
                    idx[i].state = 'U';
                    staleidx = true;
                }
            }else if(idx[i].state == 'A' && !checkedin(&idx[i], true))
                show(o, Aflg, astr, idx[i].path);
            else if(!samedata(&idx[i], &wdir[j]))
                show(o, Mflg, mstr, idx[i].path);
            else
                show(o, Tflg, tstr, idx[i].path);
            /*e: [[main()]](walk.c) when walking and exists in both index and disk */
            i++;
            j++;
        /* only exists in index */
        }else if(c < 0){
            /*s: [[main()]](walk.c) when walking and exists only in index */
            if(checkedin(&idx[i], false))
                show(o, Rflg, rstr, idx[i].path);
            else{
                idx[i].state = 'U';
                staleidx = true;
            }
            /*e: [[main()]](walk.c) when walking and exists only in index */
            i++;
        /* only exists on disk */
        }else{
            /*s: [[main()]](walk.c) when walking and exists only on disk */
            if(checkedin(&wdir[j], false)){
                if(samedata(nil, &wdir[j]))
                    show(o, Tflg, tstr, wdir[j].path);
                else
                    show(o, Mflg, mstr, wdir[j].path);
            }else if(printflg & Uflg && pfxmatch(wdir[j].path, argrel, argn, argc))
                show(o, Uflg, ustr, wdir[j].path);
            /*e: [[main()]](walk.c) when walking and exists only on disk */
            j++;
        }
    }
    /*e: [[main()]](walk.c) walking [[idx]] and [[wdir]] */
    Bterm(o);

    if(isindexed && staleidx)
       /*s: [[main()]](walk.c) finalizations, when [[isindexed && staleidx]] */
       if((wfd = create(".git/INDEX9.new", OWRITE, 0644)) != ERROR_NEG1){
           if((w = Bfdopen(wfd, OWRITE)) == nil){
               close(wfd);
               goto Nope;
           }
           for(i = 0; i < nidx; i++){
               while(i+1 < nidx && strcmp(idx[i].path, idx[i+1].path) == 0)
                   i++;
               if(idx[i].state == 'U')
                   continue;
               Bprint(w, "%c %Q %o %s\n",
                   idx[i].state,
                   idx[i].qid, 
                   idx[i].mode,
                   idx[i].path);
           }
           Bterm(w);
           nulldir(&rn);
           rn.name = "INDEX9";
           if(remove(".git/INDEX9") == -1)
               if(access(".git/INDEX9", AEXIST) == 0)
                   goto Nope;
           if(dirwstat(".git/INDEX9.new", &rn) == -1)
               sysfatal("rename: %r");
       }
       /*e: [[main()]](walk.c) finalizations, when [[isindexed && staleidx]] */

Nope:
    /*s: [[main()]](walk.c) after Nope label */
    if(!dirty)
        exits(nil);

    p = xbuf;
    e = p + sizeof(xbuf);
    for(i = 0; (1 << i) != Tflg; i++)
        if(dirty & (1 << i))
            p = seprint(p, e, "%c", "RMAUT"[i]);
    *p = '\0';
    exits(xbuf);
    /*e: [[main()]](walk.c) after Nope label */
}
/*e: function [[main (git9/walk.c)]] */
/*e: git9/walk.c */
