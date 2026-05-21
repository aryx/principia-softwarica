/*s: git9/query.c */
#include <u.h>
#include <libc.h>

#include "git.h"

#pragma varargck    type    "P" void

// flags
/*s: global [[fullpath]](query.c) */
bool fullpath;
/*e: global [[fullpath]](query.c) */
/*s: global [[changes]](query.c) */
bool changes;
/*e: global [[changes]](query.c) */
/*s: global [[reverse]](query.c) */
bool reverse;
/*e: global [[reverse]](query.c) */

/*s: global [[path]](query.c) */
char *path[128];
/*e: global [[path]](query.c) */
/*s: global [[npath]](query.c) */
int npath;
/*e: global [[npath]](query.c) */
/*s: function [[Pfmt]] */
int
Pfmt(Fmt *f)
{
    int i, n;

    n = 0;
    for(i = 0; i < npath; i++)
        n += fmtprint(f, "%s/", path[i]);
    return n;
}
/*e: function [[Pfmt]] */

/*s: function [[showdir]] */
/// show -> <>
void
showdir(Hash dh, char *dname, char m)
{
    Dirent *p, *e;
    Object *d;

    path[npath++] = dname;
    if((d = readobject(dh)) == nil)
        sysfatal("bad hash %H", dh);
    assert(d->type == GTree);
    p = d->tree->ent;
    e = p + d->tree->nent;
    for(; p != e; p++){
        if(p->ismod)
            continue;
        if(p->mode & DMDIR)
            showdir(p->h, p->name, m);
        else
            print("%c %P%s\n", m, p->name);
    }
    print("%c %P\n", m);
    unref(d);
    npath--;
}
/*e: function [[showdir]] */
/*s: function [[show (git9/query.c)]] */
void
show(Dirent *e, char m)
{
    if(e->mode & DMDIR)
        showdir(e->h, e->name, m);
    else
        print("%c %P%s\n", m, e->name);
}
/*e: function [[show (git9/query.c)]] */

/*s: function [[difftrees]] */
void
difftrees(Object *a, Object *b)
{
    Dirent *ap, *bp, *ae, *be;
    int c;

    ap = ae = nil;
    bp = be = nil;
    if(a != nil){
        if(a->type != GTree)
            return;
        ap = a->tree->ent;
        ae = ap + a->tree->nent;
    }
    if(b != nil){
        if(b->type != GTree)
            return;
        bp = b->tree->ent;
        be = bp + b->tree->nent;
    }
    while(ap != ae && bp != be){
        c = entcmp(ap, bp);
        if(c == 0){
            if(ap->mode == bp->mode && hasheq(&ap->h, &bp->h))
                goto next;
            if(ap->mode != bp->mode)
                print("! %P%s\n", ap->name);
            else if(!(ap->mode & DMDIR) || !(bp->mode & DMDIR))
                print("@ %P%s\n", ap->name);
            if((ap->mode & DMDIR) && (bp->mode & DMDIR)){
                if(npath >= nelem(path))
                    sysfatal("path too deep");
                path[npath++] = ap->name;
                if((a = readobject(ap->h)) == nil)
                    sysfatal("bad hash %H", ap->h);
                if((b = readobject(bp->h)) == nil)
                    sysfatal("bad hash %H", bp->h);
                difftrees(a, b);
                unref(a);
                unref(b);
                npath--;
            }
next:
            ap++;
            bp++;
        }else if(c < 0) {
            show(ap, '-');
            ap++;
        }else if(c > 0){
            show(bp, '+');
            bp++;
        }
    }
    for(; ap != ae; ap++)
        show(ap, '-');
    for(; bp != be; bp++)
        show(bp, '+');
}
/*e: function [[difftrees]] */
/*s: function [[diffcommits]] */
void
diffcommits(Hash ah, Hash bh)
{
    Object *a, *b, *at, *bt;

    at = nil;
    bt = nil;
    if(!hasheq(&ah, &Zhash) && (a = readobject(ah)) != nil){
        if(a->type != GCommit)
            sysfatal("not commit: %H", ah);
        if((at = readobject(a->commit->tree)) == nil)
            sysfatal("bad hash %H", a->commit->tree);
        unref(a);
    }
    if(!hasheq(&bh, &Zhash) && (b = readobject(bh)) != nil){
        if(b->type != GCommit)
            sysfatal("not commit: %H", ah);
        if((bt = readobject(b->commit->tree)) == nil)
            sysfatal("bad hash %H", b->commit->tree);
        unref(b);
    }
    difftrees(at, bt);
    unref(at);
    unref(bt);
}
/*e: function [[diffcommits]] */

/*s: function [[usage (git9/query.c)]] */
void
usage(void)
{
    fprint(STDERR, "usage: %s [-pcr] query\n", argv0);
    exits("usage");
}
/*e: function [[usage (git9/query.c)]] */
/*s: function [[main (git9/query.c)]] */
void
main(int argc, char **argv)
{
    char repo[512];
    int nrel;
    // ref_own<string>, question
    char *query;
    // array<Hash>, answer
    Hash *h;
    /*s: [[main()]](query.c) other locals */
    char *p, *e;
    int i, j, n;
    /*x: [[main()]](query.c) other locals */
    char *objpfx;
    /*e: [[main()]](query.c) other locals */

    ARGBEGIN{
    /*s: [[main()]](query.c) command line processing */
    case 'c':   changes=true;  break;
    /*x: [[main()]](query.c) command line processing */
    case 'p':   fullpath=true; break;
    /*x: [[main()]](query.c) command line processing */
    case 'r':   reverse ^= 1;   break;
    /*x: [[main()]](query.c) command line processing */
    case 'd':   chattygit++;    break;
    /*e: [[main()]](query.c) command line processing */
    default:    usage();    break;
    }ARGEND;
    if(argc == 0)
        usage();

    /*s: [[main()]](query.c) fmtinstall */
    fmtinstall('P', Pfmt);
    /*e: [[main()]](query.c) fmtinstall */

    gitinit(repo, sizeof(repo), &nrel);
    // !! chdir !!
    if(chdir(repo) == ERROR_NEG1)
        sysfatal("chdir: %r");

    /*s: [[main()]](query.c) set [[objpfx]] */
    if((objpfx = smprint("%s/.git/fs/object/", repo)) == nil)
        sysfatal("smprint: %r");
    /*e: [[main()]](query.c) set [[objpfx]] */

    // query
    /*s: [[main()]](query.c) set [[query]] derived from argv */
    // p = concat(argv, " ")
    for(i = 0, n = 0; i < argc; i++)
        n += strlen(argv[i]) + 1;
    query = emalloc(n+1);
    p = query;
    e = query + n;
    for(i = 0; i < argc; i++)
        p = seprint(p, e, "%s ", argv[i]);
    /*e: [[main()]](query.c) set [[query]] derived from argv */
    // answer
    n = resolverefs(&h, query);
    free(query);
    /*s: [[main()]](query.c) sanity check [[n]] result */
    if(n == ERROR_NEG1)
        sysfatal("resolve: %r");
    /*e: [[main()]](query.c) sanity check [[n]] result */

    /*s: [[main()]](query.c) if [[changes]] */
    if(changes){
        for(i = 1; i < n; i++)
            diffcommits(h[0], h[i]);
    }
    /*e: [[main()]](query.c) if [[changes]] */
    else{
        p = "";
        /*s: [[main()]](query.c) adjust [[p]] if [[fullpath]] */
        if (fullpath)
           p = objpfx;
        /*e: [[main()]](query.c) adjust [[p]] if [[fullpath]] */
        for(j = 0; j < n; j++) {
            i = j;
            /*s: [[main()]](query.c) adjust [[i]] if [[reverse]] */
            if (reverse)
               i = n - 1 - j;
            /*e: [[main()]](query.c) adjust [[i]] if [[reverse]] */
            print("%s%H\n", p, h[i]);
        }
    }
    exits(nil);
}
/*e: function [[main (git9/query.c)]] */
/*e: git9/query.c */
