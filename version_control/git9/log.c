/*s: git9/log.c */
#include <u.h>
#include <libc.h>
#include "git.h"

typedef struct Pfilt Pfilt;
/*s: strucr [[Pfilt]] */
struct Pfilt {
    char    *elt;
    bool show;

    Pfilt   *sub;
    int nsub;

};
/*e: strucr [[Pfilt]] */

/*s: global [[out]](log.c) */
Biobuf  *out;
/*e: global [[out]](log.c) */
/*s: global [[queryexpr]](log.c) */
char    *queryexpr;
/*e: global [[queryexpr]](log.c) */
/*s: global [[commitid]](log.c) */
// option<ref_own<string>> (if None then defaults to "HEAD")
char    *commitid;
/*e: global [[commitid]](log.c) */
/*s: global [[shortlog]](log.c) */
bool shortlog;
/*e: global [[shortlog]](log.c) */
/*s: global [[msgcount]](log.c) */
int msgcount = -1; // infinite
/*e: global [[msgcount]](log.c) */

/*s: global [[done]](log.c) */
Objset  done;
/*e: global [[done]](log.c) */
/*s: global [[objq]](log.c) */
Objq    objq;
/*e: global [[objq]](log.c) */

/*s: global [[pathfilt]](log.c) */
// option<ref_own<Pfilt>>
Pfilt   *pathfilt;
/*e: global [[pathfilt]](log.c) */

/*s: function [[filteradd]] */
void
filteradd(Pfilt *pf, char *path)
{
    char *p, *e;
    int i;

    if((e = strchr(path, '/')) != nil)
        p = smprint("%.*s", (int)(e - path), path);
    else
        p = strdup(path);

    while(e != nil && *e == '/')
        e++;
    for(i = 0; i < pf->nsub; i++){
        if(strcmp(pf->sub[i].elt, p) == 0){
            pf->sub[i].show = pf->sub[i].show || (e == nil);
            if(e != nil)
                filteradd(&pf->sub[i], e);
            free(p);
            return;
        }
    }
    pf->sub = earealloc(pf->sub, pf->nsub+1, sizeof(Pfilt));

    pf->sub[pf->nsub].elt = p;
    pf->sub[pf->nsub].show = (e == nil);
    pf->sub[pf->nsub].nsub = 0;
    pf->sub[pf->nsub].sub = nil;
    if(e != nil)
        filteradd(&pf->sub[pf->nsub], e);
    pf->nsub++;
}
/*e: function [[filteradd]] */

/*s: function [[lookup (git9/log.c)]] */
Hash
lookup(Pfilt *pf, Object *o)
{
    int i;

    if(o == nil)
        return Zhash;
    for(i = 0; i < o->tree->nent; i++)
        if(strcmp(o->tree->ent[i].name, pf->elt) == 0)
            return o->tree->ent[i].h;
    return Zhash;
}
/*e: function [[lookup (git9/log.c)]] */

/*s: function [[matchesfilter1]] */
bool
matchesfilter1(Pfilt *pf, Object *t, Object *pt)
{
    Object *a, *b;
    Hash ha, hb;
    int i, r;

    if(pf->show)
        return true;
    if(t != nil){
        if(pt != nil && t->type != pt->type)
            return true;
        if(t->type != GTree)
            return false;
    }

    for(i = 0; i < pf->nsub; i++){
        ha = lookup(&pf->sub[i], t);
        hb = lookup(&pf->sub[i], pt);
        if(hasheq(&ha, &hb))
            continue;
        a = readobject(ha);
        b = readobject(hb);
        r = matchesfilter1(&pf->sub[i], a, b);
        unref(a);
        unref(b);
        if(r)
            return true;
    }
    return false;
}
/*e: function [[matchesfilter1]] */
/*s: function [[matchesfilter]] */
bool
matchesfilter(Object *o)
{
    Object *t, *p, *pt;
    int i;
    bool r;

    assert(o->type == GCommit);
    if(pathfilt == nil)
        return true;
    t = readobject(o->commit->tree);
    if(t == nil)
        sysfatal("read %H: %r", o->commit->tree);
    for(i = 0; i < o->commit->nparent; i++){
        p = readobject(o->commit->parent[i]);
        if(p == nil)
            sysfatal("read %H: %r", o->commit->parent[i]);
        pt = readobject(p->commit->tree);
        if(pt == nil)
            sysfatal("read %H: %r", o->commit->tree);
        r = matchesfilter1(pathfilt, t, pt);
        unref(p);
        unref(pt);
        if(r)
            return true;
    }
    if(o->commit->nparent == 0)
        return matchesfilter1(pathfilt, t, nil);
    return false;
}
/*e: function [[matchesfilter]] */

/*s: function [[nextline]] */
static char*
nextline(char *p, char *e)
{
    for(; p != e; p++)
        if(*p == '\n')
            break;
    return p;
}
/*e: function [[nextline]] */

/*s: function [[show]] */
/// showcommits -> <>
static void
show(Object *o)
{
    char *p, *q, *e;

    assert(o->type == GCommit);
    /*s: [[show()]] if [[shortlog]] */
    if(shortlog){
        p = o->commit->msg;
        e = p + o->commit->nmsg;
        q = nextline(p, e);
        Bprint(out, "%H ", o->hash);
        Bwrite(out, p, q - p);
        Bputc(out, '\n');
    }
    /*e: [[show()]] if [[shortlog]] */
    else{
        Bprint(out, "Hash:\t%H\n", o->hash);
        Bprint(out, "Author:\t%s\n", o->commit->author);
        if(o->commit->committer != nil
        && strcmp(o->commit->author, o->commit->committer) != 0)
            Bprint(out, "Committer:\t%s\n", o->commit->committer);
        Bprint(out, "Date:\t%s", ctime(o->commit->mtime));
        Bprint(out, "\n");
        p = o->commit->msg;
        e = p + o->commit->nmsg;
        for(; p != e; p = q){
            q = nextline(p, e);
            Bputc(out, '\t');
            Bwrite(out, p, q - p);
            Bputc(out, '\n');
            if(q != e)
                q++;
        }
        Bprint(out, "\n");
    }
    Bflush(out);
    return;
}
/*e: function [[show]] */
/*s: function [[showquery]] */
static void
showquery(char *q)
{
    Object *o;
    Hash *h;
    int n, i;

    n = resolverefs(&h, q);
    if(n == -1)
        sysfatal("resolve: %r");

    for(i = 0; i < n && (msgcount == -1 || msgcount > 0); i++){
        o = readobject(h[i]);
        if(o == nil)
            sysfatal("read %H: %r", h[i]);
        if(matchesfilter(o)){
            show(o);
            if(msgcount != -1)
                msgcount--;
        }
        unref(o);
    }
    exits(nil);
}
/*e: function [[showquery]] */
/*s: function [[showcommits]] */
/// main(log.c) -> <>
static void
showcommits(char *c)
{
    Object *o, *p;
    Qelt e;
    int i;
    Hash h;

    if(c == nil)
        c = "HEAD";
    if(resolveref(&h, c) == -1)
        sysfatal("resolve %s: %r", c);
    o = readobject(h);
    /*s: [[showcommits()]] sanity check [[o]] is a valid commit object */
    if(o == nil)
        sysfatal("load %H: %r", h);
    if(o->type != GCommit)
        sysfatal("%s: not a commit", c);
    /*e: [[showcommits()]] sanity check [[o]] is a valid commit object */

    qinit(&objq);
    osinit(&done);
    qput(&objq, o, 0);

    while(qpop(&objq, &e) && (msgcount == -1 || msgcount > 0)){
        if(matchesfilter(e.o)){
            show(e.o);
            if(msgcount != -1)
                msgcount--;
        }
        for(i = 0; i < e.o->commit->nparent; i++){
            if(oshas(&done, e.o->commit->parent[i]))
                continue;
            // else
            p = readobject(e.o->commit->parent[i]);
            if(p == nil)
                sysfatal("load %H: %r", o->commit->parent[i]);
            osadd(&done, p);
            qput(&objq, p, 0);
        }
        unref(e.o);
    }
}
/*e: function [[showcommits]] */

/*s: function [[usage (git9/log.c)]] */
static void
usage(void)
{
    fprint(STDERR, "usage: %s [-s] [-e expr | -c commit] files..\n", argv0);
    exits("usage");
}
/*e: function [[usage (git9/log.c)]] */
/*s: function [[main (git9/log.c)]] */
void
main(int argc, char **argv)
{
    char repo[1024];
    int nrel, nrepo;
    /*s: [[main()]](log.c) other locals */
    char path[1024];
    char *p;
    char *r;
    int i;
    /*e: [[main()]](log.c) other locals */

    ARGBEGIN{
    /*s: [[main()]](log.c) command line processing */
    case 'c':
        commitid = EARGF(usage());
        break;
    /*x: [[main()]](log.c) command line processing */
    case 'n':
        msgcount = atoi(EARGF(usage()));
        break;
    /*x: [[main()]](log.c) command line processing */
    case 's':
        shortlog=true;
        break;
    /*x: [[main()]](log.c) command line processing */
    case 'e':
        queryexpr = EARGF(usage());
        break;
    /*e: [[main()]](log.c) command line processing */
    default: usage(); break;
    }ARGEND;

    gitinit(repo, sizeof(repo), &nrel);
    nrepo = strlen(repo);

    /*s: [[main()]](log.c) if [[argc]] */
    if(argc != 0){
        if(getwd(path, sizeof(path)) == nil)
            sysfatal("getwd: %r");
        if(strncmp(path, repo, nrepo) != 0)
            sysfatal("path shifted??");

        p = path + nrepo;
        pathfilt = emalloc(sizeof(Pfilt));
        for(i = 0; i < argc; i++){
            if(*argv[i] == '/'){
                if(strncmp(argv[i], repo, nrepo) != 0)
                    continue;
                r = smprint("./%s", argv[i]+nrepo);
            }else
                r = smprint("./%s/%s", p, argv[i]);
            cleanname(r);
            filteradd(pathfilt, r);
            free(r);
        }
    }
    /*e: [[main()]](log.c) if [[argc]] */
    // !!chdir!!
    if(chdir(repo) == ERROR_NEG1)
        sysfatal("chdir: %r");

    out = Bfdopen(STDOUT, OWRITE);

    /*s: [[main()]](log.c) if [[queryexpr]] */
    if(queryexpr != nil)
        showquery(queryexpr);
    /*e: [[main()]](log.c) if [[queryexpr]] */
    else
        showcommits(commitid);

    Bterm(out);
    exits(nil);
}
/*e: function [[main (git9/log.c)]] */
/*e: git9/log.c */
