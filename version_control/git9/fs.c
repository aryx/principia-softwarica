/*s: git9/fs.c */
#include <u.h>
#include <libc.h>
#include <ctype.h>

#include <fcall.h>
#include <thread.h>
#include <9p.h>

#include "git.h"

/*s: emum [[Qxxx]] */
enum Qxxx {
    Qroot,

    Qhead,
    Qbranch,
    Qobject,
    Qctl,

    // object/
    Qtree,
    Qcommit,
        Qmsg,
        Qparent,
        Qhash,
        Qauthor,
        Qcommitter,

    Qmax,
    Internal=1<<7,
};
/*e: emum [[Qxxx]] */

typedef struct Gitaux Gitaux;
typedef struct Crumb Crumb;
typedef struct Cache Cache;
typedef struct Uqid Uqid;

/*s: struct [[Crumb]] */
struct Crumb {
    char    *name;

    Qid qid;
    int mode;
    vlong   mtime;

    // option<shared_ref<Object>>
    Object  *obj;
};
/*e: struct [[Crumb]] */
/*s: struct [[Gitaux]] */
struct Gitaux {
    int ncrumb;
    // growing_array<ref_own<Crumb>> len = ncrumb
    Crumb   *crumb;

    /*s: [[Gitaux]] object dir fields */
    /* For listing object dir */
    Objlist *ols;
    Object  *olslast;
    /*e: [[Gitaux]] object dir fields */
    /*s: [[Gitaux]] other fields */
    // option<ref_own<string>>, ex: ".git/refs/" for Qbranch
    char    *refpath;
    int qdir;
    /*e: [[Gitaux]] other fields */
};
/*e: struct [[Gitaux]] */

/*s: struct [[Uqid]] */
struct Uqid {
    vlong   uqid;

    vlong   ppath;
    vlong   oid;

    int t;
    int idx;
};
/*e: struct [[Uqid]] */
/*s: struct [[Cache]] */
struct Cache {
    // growing_array<ref_own<Uqid>> (len = n)
    Uqid *cache;
    int n;
    int max;
};
/*e: struct [[Cache]] */

/*s: constant [[qroot]] */
char *qroot[] = {
    "HEAD",
    "branch",
    "object",
    "ctl",
};
/*e: constant [[qroot]] */

/*s: constant [[Eperm]] */
#define Eperm   "permission denied"
/*e: constant [[Eperm]] */
/*s: constant [[Eexist]] */
#define Eexist  "file does not exist"
/*e: constant [[Eexist]] */
/*s: constant [[Enotdir]] */
#define Enotdir "is not a directory"
/*e: constant [[Enotdir]] */
/*s: constant [[E2long]] */
#define E2long  "path too long"
/*e: constant [[E2long]] */
/*s: constant [[Enodir]] */
#define Enodir  "not a directory"
/*e: constant [[Enodir]] */
/*s: constant [[Erepo]] */
#define Erepo   "unable to read repo"
/*e: constant [[Erepo]] */
/*s: constant [[Eimpl]] */
#define Eimpl   "not implemented"
/*e: constant [[Eimpl]] */
/*s: constant [[Egreg]] */
#define Egreg   "wat"
/*e: constant [[Egreg]] */
/*s: constant [[Ebadobj]] */
#define Ebadobj "invalid object"
/*e: constant [[Ebadobj]] */

/*s: global [[gitdir]] */
char    gitdir[512];
/*e: global [[gitdir]] */
/*s: global [[username]] */
char    *username;
/*e: global [[username]] */
/*s: global [[groupname]] */
char    *groupname;
/*e: global [[groupname]] */
/*s: global [[mntpt]] */
// can be changed by git/fs -m
char    *mntpt = ".git/fs";
/*e: global [[mntpt]] */
/*s: global [[branches]] */
// growing_array<ref_own<string>>
char    **branches = nil;
/*e: global [[branches]] */
/*s: global [[uqidcache]] */
// hash<qid, Uqid>
Cache   uqidcache[512];
/*e: global [[uqidcache]] */
/*s: global [[nextqid]] */
vlong   nextqid = Qmax;
/*e: global [[nextqid]] */

static Object*  walklink(Gitaux *, char *, int, int, int*);

/*s: function [[qpath]] */
vlong
qpath(Crumb *p, int idx, vlong id, vlong t)
{
    int h, i;
    vlong pp;
    Cache *c;
    Uqid *u;

    pp = p ? p->qid.path : 0;
    h = (pp*333 + id*7 + t) & (nelem(uqidcache) - 1);
    c = &uqidcache[h];
    u = c->cache;
    for(i=0; i <c->n ; i++){
        if(u->ppath == pp && u->oid == id && u->t == t && u->idx == idx)
            return (u->uqid << 8) | t;
        u++;
    }
    if(c->n == c->max){
        c->max += c->max/2 + 1;
        c->cache = erealloc(c->cache, c->max*sizeof(Uqid));
    }
    nextqid++;
    c->cache[c->n] = (Uqid){nextqid, pp, id, t, idx};
    c->n++;
    return (nextqid << 8) | t;
}
/*e: function [[qpath]] */

/*s: function [[crumb]] */
static Crumb*
crumb(Gitaux *aux, int n)
{
    if(n < aux->ncrumb)
        return &aux->crumb[aux->ncrumb - n - 1];
    return nil;
}
/*e: function [[crumb]] */

/*s: function [[popcrumb]] */
static void
popcrumb(Gitaux *aux)
{
    Crumb *c;

    if(aux->ncrumb > 1){
        c = crumb(aux, 0);
        free(c->name);
        unref(c->obj);
        aux->ncrumb--;
    }
}
/*e: function [[popcrumb]] */

/*s: function [[branchid]] */
static vlong
branchid(Gitaux *aux, char *path)
{
    int i;

    for(i = 0; branches[i]; i++)
        if(strcmp(path, branches[i]) == 0)
            goto found;
    branches = realloc(branches, sizeof(char *)*(i + 2));
    branches[i] = estrdup(path);
    branches[i + 1] = nil;

found:
    if(aux){
        if(aux->refpath)
            free(aux->refpath);
        aux->refpath = estrdup(branches[i]);
    }
    return i;
}
/*e: function [[branchid]] */

/*s: function [[obj2dir]] */
static void
obj2dir(Dir *d, Crumb *c, Object *o, char *name)
{
    d->qid = c->qid;
    d->atime = c->mtime;
    d->mtime = c->mtime;
    d->mode = c->mode;

    d->name = estrdup9p(name);
    d->uid = estrdup9p(username);
    d->gid = estrdup9p(groupname);
    d->muid = estrdup9p(username);

    if(o->type == GBlob || o->type == GTag){
        d->qid.type = 0;
        d->mode &= 0777;
        d->length = o->size;
    }

}
/*e: function [[obj2dir]] */

/*s: function [[rootgen]] */
static errorneg1
rootgen(int i, Dir *d, void *p)
{
    Crumb *c;

    c = crumb(p, 0);
    if (i >= nelem(qroot))
        return ERROR_NEG1;

    d->mode = 0555 | DMDIR;
    d->name = estrdup9p(qroot[i]);
    d->qid.vers = 0;
    d->qid.type = strcmp(qroot[i], "ctl") == 0 ? 0 : QTDIR;
    d->qid.path = qpath(nil, i, i, Qroot);
    d->uid = estrdup9p(username);
    d->gid = estrdup9p(groupname);
    d->muid = estrdup9p(username);
    d->mtime = c->mtime;
    return OK_0;
}
/*e: function [[rootgen]] */

/*s: function [[branchgen]] */
static int
branchgen(int i, Dir *d, void *p)
{
    Gitaux *aux;
    Dir *refs;
    Crumb *c;
    int n;

    aux = p;
    c = crumb(aux, 0);
    refs = nil;
    d->qid.vers = 0;
    d->qid.type = QTDIR;
    d->qid.path = qpath(c, i, branchid(aux, aux->refpath), Qbranch | Internal);
    d->mode = 0555 | DMDIR;
    d->uid = estrdup9p(username);
    d->gid = estrdup9p(groupname);
    d->muid = estrdup9p(username);
    d->mtime = c->mtime;
    d->atime = c->mtime;
    if((n = slurpdir(aux->refpath, &refs)) < 0)
        return -1;
    if(i < n){
        d->name = estrdup9p(refs[i].name);
        free(refs);
        return 0;
    }else{
        free(refs);
        return -1;
    }
}
/*e: function [[branchgen]] */
/*s: function [[gtreegen]] */
static int
gtreegen(int i, Dir *d, void *p)
{
    Object *o, *l, *e;
    Gitaux *aux;
    Crumb *c;
    int m;

    aux = p;
    c = crumb(aux, 0);
    e = c->obj;
    if(i >= e->tree->nent)
        return ERROR_NEG1;
    m = e->tree->ent[i].mode;
    /*s: [[gtreegen()]] if submodule */
    if(e->tree->ent[i].ismod)
        o = emptydir();
    /*e: [[gtreegen()]] if submodule */
    else if((o = readobject(e->tree->ent[i].h)) == nil)
        sysfatal("could not read object %H: %r", e->tree->ent[i].h);

    /*s: [[gtreegen()]] if [[e->tree->ent[i].islink]] */
    if(e->tree->ent[i].islink)
        if((l = walklink(aux, o->data, o->size, 0, &m)) != nil)
            o = l;
    /*e: [[gtreegen()]] if [[e->tree->ent[i].islink]] */

    d->qid.vers = 0;
    d->qid.type = o->type == GTree ? QTDIR : 0;
    d->qid.path = qpath(c, i, o->id, aux->qdir);
    d->mode = m;
    d->atime = c->mtime;
    d->mtime = c->mtime;
    d->uid = estrdup9p(username);
    d->gid = estrdup9p(groupname);
    d->muid = estrdup9p(username);
    d->name = estrdup9p(e->tree->ent[i].name);
    d->length = o->size;
    return OK_0;
}
/*e: function [[gtreegen]] */
/*s: function [[gcommitgen]] */
static int
gcommitgen(int i, Dir *d, void *p)
{
    Object *o;
    Crumb *c;

    c = crumb(p, 0);
    o = c->obj;

    d->uid = estrdup9p(username);
    d->gid = estrdup9p(groupname);
    d->muid = estrdup9p(username);
    d->mode = 0444;
    d->atime = o->commit->ctime;
    d->mtime = o->commit->ctime;
    d->qid.type = 0;
    d->qid.vers = 0;

    switch(i){
    /*s: [[gcommitgen()]] switch [[i]] cases */
    case 0:
        d->mode = DMDIR | gitdirmode;
        d->name = estrdup9p("tree");
        d->qid.type = QTDIR;
        d->qid.path = qpath(c, i, o->id, Qtree);
        break;
    /*x: [[gcommitgen()]] switch [[i]] cases */
    case 1:
        d->name = estrdup9p("parent");
        d->qid.path = qpath(c, i, o->id, Qparent);
        break;
    /*x: [[gcommitgen()]] switch [[i]] cases */
    case 2:
        d->name = estrdup9p("msg");
        d->qid.path = qpath(c, i, o->id, Qmsg);
        break;
    /*x: [[gcommitgen()]] switch [[i]] cases */
    case 3:
        d->name = estrdup9p("hash");
        d->qid.path = qpath(c, i, o->id, Qhash);
        break;
    /*x: [[gcommitgen()]] switch [[i]] cases */
    case 4:
        d->name = estrdup9p("author");
        d->qid.path = qpath(c, i, o->id, Qauthor);
        break;
    /*e: [[gcommitgen()]] switch [[i]] cases */
    default:
        free(d->uid);
        free(d->gid);
        free(d->muid);
        return ERROR_NEG1;
    }
    // else
    return OK_0;
}
/*e: function [[gcommitgen]] */

/*s: function [[objgen]] */
static int
objgen(int i, Dir *d, void *p)
{
    Gitaux *aux;
    Object *o;
    Crumb *c;
    char name[64];
    Objlist *ols;
    Hash h;

    aux = p;
    c = crumb(aux, 0);

    if(!aux->ols)
        aux->ols = mkols();
    ols = aux->ols;
    o = nil;

    /* We tried to sent it, but it didn't fit */
    if(aux->olslast && ols->idx == i + 1){
        snprint(name, sizeof(name), "%H", aux->olslast->hash);
        obj2dir(d, c, aux->olslast, name);
        return OK_0;
    }
    // else
    while(ols->idx <= i){
        if(olsnext(ols, &h) == -1)
            return ERROR_NEG1;
        if((o = readobject(h)) == nil){
            fprint(STDERR, "corrupt object %H\n", h);
            return ERROR_NEG1;
        }
    }
    if(o != nil){
        snprint(name, sizeof(name), "%H", o->hash);
        obj2dir(d, c, o, name);
        unref(aux->olslast);
        aux->olslast = ref(o);
        return OK_0;
    }
    // else
    return ERROR_NEG1;
}
/*e: function [[objgen]] */
/*s: function [[objread]] */
static void
objread(Req *r, Gitaux *aux)
{
    Object *o;

    o = crumb(aux, 0)->obj;
    switch(o->type){
    /*s: [[objread()]] switch [[o->type]] cases */
    case GBlob:
        readbuf(r, o->data, o->size);
        break;
    /*x: [[objread()]] switch [[o->type]] cases */
    case GTree:
        dirread9p(r, gtreegen, aux);
        break;
    /*x: [[objread()]] switch [[o->type]] cases */
    case GCommit:
        dirread9p(r, gcommitgen, aux);
        break;
    /*x: [[objread()]] switch [[o->type]] cases */
    case GTag:
        readbuf(r, o->data, o->size);
        break;
    /*e: [[objread()]] switch [[o->type]] cases */
    default:
        sysfatal("invalid object type %d", o->type);
    }
}
/*e: function [[objread]] */

/*s: function [[readcommitparent]] */
static void
readcommitparent(Req *r, Object *o)
{
    char *buf, *p, *e;
    int i, n;

    /* 40 bytes per hash, 1 per nl, 1 for terminator */
    n = o->commit->nparent * (40 + 1) + 1;
    buf = emalloc(n);
    p = buf;
    e = buf + n;
    for (i = 0; i < o->commit->nparent; i++)
        p = seprint(p, e, "%H\n", o->commit->parent[i]);
    readbuf(r, buf, p - buf);
    free(buf);
}
/*e: function [[readcommitparent]] */

/*s: function [[gitattach]] */
static void
gitattach(Req *r)
{
    Gitaux *aux;
    Dir *d;

    d = dirstat(".git");
    if(d == nil)
        sysfatal("git/fs: %r");
    if(getwd(gitdir, sizeof(gitdir)) == nil)
        sysfatal("getwd: %r");

    aux = emalloc(sizeof(Gitaux));
    aux->crumb = emalloc(sizeof(Crumb));
    aux->ncrumb = 1;
    /*s: [[gitattach()]] set [[aux->crumb]] */
    aux->crumb[0].qid = (Qid){Qroot, 0, QTDIR};
    aux->crumb[0].obj = nil;
    aux->crumb[0].mode = DMDIR | 0555;
    aux->crumb[0].mtime = d->mtime;
    aux->crumb[0].name = estrdup("/");
    /*e: [[gitattach()]] set [[aux->crumb]] */

    r->ofcall.qid = (Qid){Qroot, 0, QTDIR};
    r->fid->qid = r->ofcall.qid;
    r->fid->aux = aux;

    respond(r, nil);
}
/*e: function [[gitattach]] */

/*s: function [[walklink]] */
static Object*
walklink(Gitaux *aux, char *link, int nlink, int ndotdot, int *mode)
{
    char *p, *e, *path;
    Object *o, *n;
    int i;

    path = emalloc(nlink + 1);
    memcpy(path, link, nlink);
    cleanname(path);

    o = crumb(aux, ndotdot)->obj;
    assert(o->type == GTree);
    for(p = path; *p; p = e){
        n = nil;
        e = p + strcspn(p, "/");
        if(*e == '/')
            *e++ = '\0';
        /*
         * cleanname guarantees these show up at the start of the name,
         * which allows trimming them from the end of the trail of crumbs
         * instead of needing to keep track of full parentage.
         */
        if(strcmp(p, "..") == 0)
            n = crumb(aux, ++ndotdot)->obj;
        else if(o->type == GTree)
            for(i = 0; i < o->tree->nent; i++)
                if(strcmp(o->tree->ent[i].name, p) == 0){
                    *mode = o->tree->ent[i].mode;
                    n = readobject(o->tree->ent[i].h);
                    break;
                }
        o = n;
        if(o == nil)
            break;
    }
    free(path);
    for(i = 0; o != nil && i < aux->ncrumb; i++)
        if(crumb(aux, i)->obj == o)
            return nil;
    return o;
}
/*e: function [[walklink]] */

/*s: function [[objwalk1]] */
static char *
objwalk1(Qid *q, Object *o, Crumb *p, Crumb *c, char *name, vlong qdir, Gitaux *aux)
{
    Object *w, *l;
    char *e;
    int i, m;

    w = nil;
    e = nil;
    if(!o)
        return Eexist;
    switch(o->type){
    /*s: [[objwalk1()]] switch [[o->type]] cases */
    case GBlob:
        e = Enotdir;
        break;
    /*x: [[objwalk1()]] switch [[o->type]] cases */
    case GTree:
        q->type = 0;
        for(i = 0; i < o->tree->nent; i++){
            if(strcmp(o->tree->ent[i].name, name) != 0)
                continue;
            m = o->tree->ent[i].mode;
            w = readobject(o->tree->ent[i].h);
        
            /*s: [[objwalk1()]] when [[GTree]] case, if [[o->tree->ent[i].ismod]] */
            if(!w && o->tree->ent[i].ismod)
                w = emptydir();
            /*e: [[objwalk1()]] when [[GTree]] case, if [[o->tree->ent[i].ismod]] */
            /*s: [[objwalk1()]] when [[GTree]] case, if [[o->tree->ent[i].islink]] */
            if(w && o->tree->ent[i].islink)
                if((l = walklink(aux, w->data, w->size, 1, &m)) != nil)
                    w = l;
            /*e: [[objwalk1()]] when [[GTree]] case, if [[o->tree->ent[i].islink]] */

            if(!w)
                return Ebadobj;
            q->type = (w->type == GTree) ? QTDIR : 0;
            q->path = qpath(p, i, w->id, qdir);
            c->mode = m;
            c->mode |= (w->type == GTree) ? (DMDIR|0755) : 0644;
            c->obj = w;
            break;
        }
        if(!w)
            e = Eexist;
        break;
    /*x: [[objwalk1()]] switch [[o->type]] cases */
    case  GCommit:
        q->type = 0;
        c->mtime = o->commit->mtime;
        c->mode = 0644;
        assert(qdir == Qcommit || qdir == Qobject || qdir == Qtree || qdir == Qhead || qdir == Qcommitter);
        if(strcmp(name, "msg") == 0)
            q->path = qpath(p, 0, o->id, Qmsg);
        else if(strcmp(name, "parent") == 0)
            q->path = qpath(p, 1, o->id, Qparent);
        else if(strcmp(name, "hash") == 0)
            q->path = qpath(p, 2, o->id, Qhash);
        else if(strcmp(name, "author") == 0)
            q->path = qpath(p, 3, o->id, Qauthor);
        else if(strcmp(name, "committer") == 0)
            q->path = qpath(p, 3, o->id, Qcommitter);
        else if(strcmp(name, "tree") == 0){
            q->type = QTDIR;
            q->path = qpath(p, 4, o->id, Qtree);
            unref(c->obj);
            c->mode = DMDIR | gitdirmode;
            c->obj = readobject(o->commit->tree);
            if(c->obj == nil)
                sysfatal("could not read object %H: %r", o->commit->tree);
        }
        else
            e = Eexist;
        break;
    /*x: [[objwalk1()]] switch [[o->type]] cases */
    case GTag:
        e = Eimpl;
        break;
    /*e: [[objwalk1()]] switch [[o->type]] cases */
    }
    return e;
}
/*e: function [[objwalk1]] */

/*s: function [[readref]] */
static Object *
readref(char *pathstr)
{
    fdt f;
    char buf[128], path[128], *p, *e;
    Hash h;
    int n;

    snprint(path, sizeof(path), "%s", pathstr);
    while(1){
        if((f = open(path, OREAD)) == -1)
            return nil;
        if((n = readn(f, buf, sizeof(buf) - 1)) == -1)
            return nil;
        close(f);
        buf[n] = '\0';
        if(strncmp(buf, "ref:", 4) !=  0)
            break;

        // else
        p = buf + 4;
        while(isspace(*p))
            p++;
        if((e = strchr(p, '\n')) != nil)
            *e = 0;
        snprint(path, sizeof(path), ".git/%s", p);
    }

    if(hparse(&h, buf) == ERROR_NEG1)
        return nil;

    return readobject(h);
}
/*e: function [[readref]] */

/*s: function [[gitwalk1]] */
static char*
gitwalk1(Fid *fid, char *name, Qid *q)
{
    char path[128];
    Gitaux *aux;
    Crumb *c, *o;
    char *e;
    Dir *d;
    Hash h;

    e = nil;
    aux = fid->aux;
    
    q->vers = 0;
    /*s: [[gitwalk1()]] if dotdot */
    if(strcmp(name, "..") == 0){
        popcrumb(aux);
        c = crumb(aux, 0);
        *q = c->qid;
        fid->qid = *q;
        return nil;
    }
    /*e: [[gitwalk1()]] if dotdot */
    // else
    
    aux->crumb = realloc(aux->crumb, (aux->ncrumb + 1) * sizeof(Crumb));
    aux->ncrumb++;

    c = crumb(aux, 0);
    o = crumb(aux, 1);
    memset(c, 0, sizeof(Crumb));
    c->mode = o->mode;
    c->mtime = o->mtime;
    c->obj = o->obj ? ref(o->obj) : nil;
    
    switch(QDIR(&fid->qid)){
    /*s: [[gitwalk1()]] switch [[QDIR(&fid->qid)]] cases */
    case Qroot:
        if(strcmp(name, "ctl") == 0){
            *q = (Qid){Qctl, 0, 0};
            c->mode = 0644;
        /*s: [[gitwalk1()]] when [[Qroot]] case, switch on [[name]] other cases */
        }else if(strcmp(name, "object") == 0){
            *q = (Qid){Qobject, 0, QTDIR};
            c->mode = DMDIR | 0555;
        /*x: [[gitwalk1()]] when [[Qroot]] case, switch on [[name]] other cases */
        }else if(strcmp(name, "HEAD") == 0){
            *q = (Qid){Qhead, 0, QTDIR};
            c->mode = DMDIR | 0555;
            c->obj = readref(".git/HEAD");
        /*x: [[gitwalk1()]] when [[Qroot]] case, switch on [[name]] other cases */
        }else if(strcmp(name, "branch") == 0){
            *q = (Qid){Qbranch, 0, QTDIR};
            aux->refpath = estrdup(".git/refs/");
            c->mode = DMDIR | 0555;
        /*e: [[gitwalk1()]] when [[Qroot]] case, switch on [[name]] other cases */
        }else{
            e = Eexist;
        }
        break;
    /*x: [[gitwalk1()]] switch [[QDIR(&fid->qid)]] cases */
    case Qctl:
        return Enodir;
    /*x: [[gitwalk1()]] switch [[QDIR(&fid->qid)]] cases */
    case Qobject:
        if(c->obj){
            e = objwalk1(q, o->obj, o, c, name, Qobject, aux);
        }else{
            if(hparse(&h, name) == ERROR_NEG1)
                return Ebadobj;
            c->obj = readobject(h);
            if(c->obj == nil)
                return Ebadobj;
            if(c->obj->type == GBlob || c->obj->type == GTag){
                c->mode = 0644;
                q->type = 0;
            }else{
                c->mode = DMDIR | 0755;
                q->type = QTDIR;
            }
            q->path = qpath(o, Qobject, c->obj->id, Qobject);
            q->vers = 0;
        }
        break;
    /*x: [[gitwalk1()]] switch [[QDIR(&fid->qid)]] cases */
    case Qtree:
        e = objwalk1(q, o->obj, o, c, name, Qtree, aux);
        break;
    /*x: [[gitwalk1()]] switch [[QDIR(&fid->qid)]] cases */
    case Qcommit:
        e = objwalk1(q, o->obj, o, c, name, Qcommit, aux);
        break;
    /*x: [[gitwalk1()]] switch [[QDIR(&fid->qid)]] cases */
    case Qmsg:
    case Qparent:
    case Qhash:
    case Qauthor:
    case Qcommitter:
        return Enodir;
    /*x: [[gitwalk1()]] switch [[QDIR(&fid->qid)]] cases */
    case Qhead:
        e = objwalk1(q, o->obj, o, c, name, Qhead, aux);
        break;
    /*x: [[gitwalk1()]] switch [[QDIR(&fid->qid)]] cases */
    case Qbranch:
        if(strcmp(aux->refpath, ".git/refs/heads") == 0 && strcmp(name, "HEAD") == 0)
            snprint(path, sizeof(path), ".git/HEAD");
        else
            snprint(path, sizeof(path), "%s/%s", aux->refpath, name);
        q->type = QTDIR;
        d = dirstat(path);
        if(d && d->qid.type == QTDIR)
            q->path = qpath(o, Qbranch, branchid(aux, path), Qbranch);
        else if(d && (c->obj = readref(path)) != nil)
            q->path = qpath(o, Qbranch, c->obj->id, Qcommit);
        else
            e = Eexist;
        if(d != nil)
            c->mode = d->mode & ~0222;
        free(d);
        break;
    /*e: [[gitwalk1()]] switch [[QDIR(&fid->qid)]] cases */
    default:
        return Egreg;
    }

    c->name = estrdup(name);
    c->qid = *q;
    fid->qid = *q;
    return e;
}
/*e: function [[gitwalk1]] */

/*s: function [[gitclone]] */
static char*
gitclone(Fid *o, Fid *n)
{
    Gitaux *aux, *oaux;
    int i;

    oaux = o->aux;
    aux = emalloc(sizeof(Gitaux));

    aux->ncrumb = oaux->ncrumb;
    aux->crumb = eamalloc(oaux->ncrumb, sizeof(Crumb));
    for(i = 0; i < aux->ncrumb; i++){
        aux->crumb[i] = oaux->crumb[i];
        aux->crumb[i].name = estrdup(oaux->crumb[i].name);
        if(aux->crumb[i].obj)
            aux->crumb[i].obj = ref(oaux->crumb[i].obj);
    }
    if(oaux->refpath)
        aux->refpath = strdup(oaux->refpath);
    aux->qdir = oaux->qdir;
    n->aux = aux;
    return nil;
}
/*e: function [[gitclone]] */

/*s: function [[gitdestroyfid]] */
static void
gitdestroyfid(Fid *f)
{
    Gitaux *aux;
    int i;

    if((aux = f->aux) == nil)
        return;
    for(i = 0; i < aux->ncrumb; i++){
        if(aux->crumb[i].obj)
            unref(aux->crumb[i].obj);
        free(aux->crumb[i].name);
    }
    olsfree(aux->ols);
    free(aux->refpath);
    free(aux->crumb);
    free(aux);
}
/*e: function [[gitdestroyfid]] */

/*s: function [[readctl]] */
static char *
readctl(Req *r)
{
    fdt fd;
    int n;
    char data[1024], ref[512];
    char *s, *e;

    fd = open(".git/HEAD", OREAD);
    /*s: [[readctl()]] sanity check [[fd]] */
    if(fd == ERROR_NEG1)
        return Erepo;
    /*e: [[readctl()]] sanity check [[fd]] */
    /* empty HEAD is invalid */
    n = readn(fd, ref, sizeof(ref) - 1);
    /*s: [[readctl()]] sanity check [[n]] */
    if(n <= 0)
        return Erepo;
    /*e: [[readctl()]] sanity check [[n]] */
    close(fd);

    s = ref;
    ref[n] = '\0';
    if(strncmp(s, "ref:", 4) == 0)
        s += 4;
    while(*s == ' ' || *s == '\t')
        s++;
    if((e = strchr(s, '\n')) != nil)
        *e = '\0';
    if(strstr(s, "refs/") == s)
        s += strlen("refs/");

    snprint(data, sizeof(data), "branch %s\nrepo %s\n", s, gitdir);
    readstr(r, data);
    return nil;
}
/*e: function [[readctl]] */

/*s: function [[gitread]] */
static void
gitread(Req *r)
{
    char buf[256], *e;
    Gitaux *aux;
    Object *o;
    Qid *q;

    aux = r->fid->aux;
    q = &r->fid->qid;
    o = crumb(aux, 0)->obj;
    e = nil;

    switch(QDIR(q)){
    /*s: [[gitread()]] switch [[QDIR(q)]] cases */
    case Qroot:
        dirread9p(r, rootgen, aux);
        break;
    /*x: [[gitread()]] switch [[QDIR(q)]] cases */
    case Qctl:
        e = readctl(r);
        break;
    /*x: [[gitread()]] switch [[QDIR(q)]] cases */
    case Qobject:
        if(o)
            objread(r, aux);
        else
            dirread9p(r, objgen, aux);
        break;
    /*x: [[gitread()]] switch [[QDIR(q)]] cases */
    case Qtree:
        objread(r, aux);
        break;
    /*x: [[gitread()]] switch [[QDIR(q)]] cases */
    case Qcommit:
        objread(r, aux);
        break;
    /*x: [[gitread()]] switch [[QDIR(q)]] cases */
    case Qparent:
        readcommitparent(r, o);
        break;
    /*x: [[gitread()]] switch [[QDIR(q)]] cases */
    case Qmsg:
        readbuf(r, o->commit->msg, o->commit->nmsg);
        break;
    /*x: [[gitread()]] switch [[QDIR(q)]] cases */
    case Qhash:
        snprint(buf, sizeof(buf), "%H\n", o->hash);
        readstr(r, buf);
        break;
    /*x: [[gitread()]] switch [[QDIR(q)]] cases */
    case Qauthor:
        snprint(buf, sizeof(buf), "%s\n", o->commit->author);
        readstr(r, buf);
        break;
    /*x: [[gitread()]] switch [[QDIR(q)]] cases */
    case Qcommitter:
        snprint(buf, sizeof(buf), "%s\n", o->commit->committer);
        readstr(r, buf);
        break;
    /*x: [[gitread()]] switch [[QDIR(q)]] cases */
    case Qhead:
        /* Empty repositories have no HEAD */
        if(o == nil)
            r->ofcall.count = 0;
        else
            objread(r, aux);
        break;
    /*x: [[gitread()]] switch [[QDIR(q)]] cases */
    case Qbranch:
        if(o)
            objread(r, aux);
        else
            dirread9p(r, branchgen, aux);
        break;
    /*e: [[gitread()]] switch [[QDIR(q)]] cases */
    default:
        e = Egreg;
    }
    respond(r, e);
}
/*e: function [[gitread]] */
/*s: function [[gitopen]] */
static void
gitopen(Req *r)
{
    Gitaux *aux;
    Crumb *c;

    aux = r->fid->aux;
    c = crumb(aux, 0);
    switch(r->ifcall.mode&3){
    case OREAD:
    case ORDWR:
        respond(r, nil);
        break;
    case OWRITE:
        respond(r, Eperm);
        break;
    case OEXEC:
        if((c->mode & 0111) == 0)
            respond(r, Eperm);
        else
            respond(r, nil);
        break;
    default:
        respond(r, "botched mode");
        break;
    }
}
/*e: function [[gitopen]] */
/*s: function [[gitstat]] */
static void
gitstat(Req *r)
{
    Gitaux *aux;
    Crumb *c;

    aux = r->fid->aux;
    c = crumb(aux, 0);

    r->d.uid = estrdup9p(username);
    r->d.gid = estrdup9p(groupname);
    r->d.muid = estrdup9p(username);

    r->d.qid = r->fid->qid;
    r->d.mtime = c->mtime;
    r->d.atime = c->mtime;
    r->d.mode = c->mode;

    if(c->obj)
        obj2dir(&r->d, c, c->obj, c->name);
    else
        r->d.name = estrdup9p(c->name);
    respond(r, nil);
}
/*e: function [[gitstat]] */

/*s: global [[gitsrv]] */
Srv gitsrv = {
    // will attach and set gitdir too
    .attach=gitattach,
    .walk1=gitwalk1,
    .clone=gitclone,
    .open=gitopen,
    .read=gitread,
    .stat=gitstat,
    .destroyfid=gitdestroyfid,
};
/*e: global [[gitsrv]] */

/*s: function [[usage (git9/fs.c)]] */
void
usage(void)
{
    fprint(STDERR, "usage: %s [-d]\n", argv0);
    fprint(STDERR, "\t-d:    debug\n");
    exits("usage");
}
/*e: function [[usage (git9/fs.c)]] */
/*s: function [[main (git9/fs.c)]] */
void
main(int argc, char **argv)
{
    char repo[512];
    int nelt;
    Dir *d;

    gitinit(repo, sizeof(repo), &nelt);
    // !!chdir!!
    if(chdir(repo) == ERROR_NEG1)
        sysfatal("chdir: %r");

    ARGBEGIN{
    /*s: [[main()]](fs.c) command line processing */
    case 'm':
        mntpt = EARGF(usage());
        break;
    /*x: [[main()]](fs.c) command line processing */
    case 'd':
        chatty9p++;
        break;
    /*e: [[main()]](fs.c) command line processing */
    default: usage(); break;
    }ARGEND;

    if(argc != 0)
        usage();

    // can use just relative .git path because of chdir() above
    d = dirstat(".git");
    if(d == nil)
        sysfatal("dirstat .git: %r");
    username = strdup(d->uid);
    groupname = strdup(d->gid);
    free(d);

    branches = emalloc(sizeof(char*));
    branches[0] = nil;
    postmountsrv(&gitsrv, nil, mntpt, MCREATE);
    exits(nil);
}
/*e: function [[main (git9/fs.c)]] */
/*e: git9/fs.c */
