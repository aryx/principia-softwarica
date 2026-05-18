/*s: git9/ref.c */
#include <u.h>
#include <libc.h>
#include <ctype.h>

#include "git.h"

typedef struct Eval Eval;

enum {
    Blank,
    Keep,
    Drop,
    Skip,
};

/*s: enum [[PaintMode]] */
enum PaintMode {
    Lca,
    Range,
    /*s: [[PaintMode]] other cases */
    Twixt,
    /*e: [[PaintMode]] other cases */
};
/*e: enum [[PaintMode]] */

/*s: struct [[Eval]] */
struct Eval {
    // ref<string> (owner = main() query)
    char    *str;
    // pointer in str as we parse it
    char    *p;

    // stack<ref_own<Object>>, len = nstk
    Object  **stk;
    int nstk;
    int stksz;
};
/*e: struct [[Eval]] */

static char *colors[] = {
[Keep] "keep",
[Drop] "drop",
[Blank] "blank",
[Skip] "skip",
};

/*s: global [[zcommit]](ref.c) */
static Object zcommit = {
    .type=GCommit
};
/*e: global [[zcommit]](ref.c) */

/*s: function [[eatspace]] */
void
eatspace(Eval *ev)
{
    while(isspace(ev->p[0]))
        ev->p++;
}
/*e: function [[eatspace]] */

/*s: function [[push]] */
void
push(Eval *ev, Object *o)
{
    /*s: [[push()]] realloc if needed */
    if(ev->nstk == ev->stksz){
        ev->stksz = 2*ev->stksz + 1;
        ev->stk = erealloc(ev->stk, ev->stksz*sizeof(Object*));
    }
    /*e: [[push()]] realloc if needed */
    ev->stk[ev->nstk++] = o;
}
/*e: function [[push]] */

/*s: function [[pop]] */
Object*
pop(Eval *ev)
{
    /*s: [[pop()]] sanity check empty stack */
    if(ev->nstk == 0)
        sysfatal("stack underflow");
    /*e: [[pop()]] sanity check empty stack */
    return ev->stk[--ev->nstk];
}
/*e: function [[pop]] */

/*s: function [[isword]] */
bool
isword(char e)
{
    return isalnum(e) || e == '/' || e == '-' || e == '_' || e == '.';
}
/*e: function [[isword]] */

/*s: function [[word]] */
bool
word(Eval *ev, char *b, int nb)
{
    char *p, *e;
    int n;

    p = ev->p;
    for(e = p; isword(*e) && strncmp(e, "..", 2) != 0; e++)
        /* nothing */;
    /* 1 for nul terminator */
    n = e - p + 1;
    if(n >= nb)
        n = nb;
    snprint(b, n, "%s", p);
    ev->p = e;
    return n > 0;
}
/*e: function [[word]] */

/*s: function [[take]] */
int
take(Eval *ev, char *m)
{
    int l;

    l = strlen(m);
    if(strncmp(ev->p, m, l) != 0)
        return 0;
    ev->p += l;
    return 1;
}
/*e: function [[take]] */

/*s: function [[paint]] */
/// lca | ancestor | range -> <>
static errorneg1
paint(Hash *head, int nhead, Hash *tail, int ntail, Object ***res, int *nres, int mode)
{
    Qelt e;
    Objq objq;
    Objset keep, drop, skip;
    Object *o, *c, **range;
    int i, nskip, nrange;

    osinit(&keep);
    osinit(&drop);
    osinit(&skip);
    qinit(&objq);
    range = nil;
    nrange = 0;
    nskip = 0;

    for(i = 0; i < nhead; i++){
        if(hasheq(&head[i], &Zhash))
            continue;
        if((o = readobject(head[i])) == nil){
            fprint(2, "warning: %H does not point at commit\n", head[i]);
            werrstr("read head %H: %r", head[i]);
            return -1;
        }
        if(o->type != GCommit){
            fprint(2, "warning: %H does not point at commit\n", o->hash);
            unref(o);
            continue;
        }
        dprint(1, "init: keep %H\n", o->hash);
        qput(&objq, o, Keep);
        unref(o);
    }       
    for(i = 0; i < ntail; i++){
        if(hasheq(&tail[i], &Zhash))
            continue;
        if((o = readobject(tail[i])) == nil){
            werrstr("read tail %H: %r", tail[i]);
            return -1;
        }
        if(o->type != GCommit){
            fprint(2, "warning: %H does not point at commit\n", o->hash);
            unref(o);
            continue;
        }
        dprint(1, "init: drop %H\n", o->hash);
        qput(&objq, o, Drop);
        unref(o);
    }

    dprint(1, "finding twixt commits\n");
    while(objq.nheap != 0 && qpop(&objq, &e)){
        if(e.color == Skip)
            nskip--;
        if(oshas(&skip, e.o->hash))
            continue;
        switch(e.color){
        case Keep:
            if(oshas(&keep, e.o->hash))
                continue;
            if(oshas(&drop, e.o->hash))
                e.color = Skip;
            else if(mode == Range){
                range = earealloc(range, nrange+1, sizeof(Object*));
                range[nrange++] = e.o;
            }
            osadd(&keep, e.o);
            break;
        case Drop:
            if(oshas(&drop, e.o->hash))
                continue;
            if(oshas(&keep, e.o->hash))
                e.color = Skip;
            osadd(&drop, e.o);
            break;
        case Skip:
            osadd(&skip, e.o);
            break;
        }
        o = readobject(e.o->hash);
        if(o->type != GCommit){
            werrstr("not a commit: %H", o->hash);
            goto error;
        }
        for(i = 0; i < o->commit->nparent; i++){
            if((c = readobject(e.o->commit->parent[i])) == nil)
                goto error;
            if(c->type != GCommit){
                fprint(2, "warning: %H does not point at commit\n", c->hash);
                unref(c);
                continue;
            }
            dprint(2, "\tenqueue: %s %H\n", colors[e.color], c->hash);
            qput(&objq, c, e.color);
            unref(c);
            if(e.color == Skip)
                nskip++;
        }
        unref(o);
    }
    switch(mode){
    case Lca:
        dprint(1, "found ancestor\n");
        o = nil;
        for(i = 0; i < keep.sz; i++){
            o = keep.obj[i];
            if(o != nil && oshas(&drop, o->hash) && !oshas(&skip, o->hash))
                break;
        }
        if(i == keep.sz){
            *nres = 0;
            *res = nil;
        }else{
            *nres = 1;
            *res = eamalloc(1, sizeof(Object*));
            (*res)[0] = o;
        }
        break;
    case Twixt:
        dprint(1, "found twixt\n");
        *res = eamalloc(keep.nobj, sizeof(Object*));
        *nres = 0;
        for(i = 0; i < keep.sz; i++){
            o = keep.obj[i];
            if(o != nil && !oshas(&drop, o->hash) && !oshas(&skip, o->hash)){
                (*res)[*nres] = o;
                (*nres)++;
            }
        }
        break;
    case Range:
        dprint(1, "found range\n");
        *res = eamalloc(nrange, sizeof(Object*));
        *nres = 0;
        for(i = nrange - 1; i >= 0; i--){
            o = range[i];
            if(!oshas(&drop, o->hash) && !oshas(&skip, o->hash)){
                (*res)[*nres] = o;
                (*nres)++;
            }
        }
        free(range);
        break;
    }
    osclear(&keep);
    osclear(&drop);
    osclear(&skip);
    return 0;
error:
    dprint(1, "paint error: %r\n");
    free(objq.heap);
    free(range);
    return -1;
}
/*e: function [[paint]] */

/*s: function [[findtwixt]] */
int
findtwixt(Hash *head, int nhead, Hash *tail, int ntail, Object ***res, int *nres)
{
    return paint(head, nhead, tail, ntail, res, nres, Twixt);
}
/*e: function [[findtwixt]] */

/*s: function [[ancestor]] */
/// sendpack -> <>
Object*
ancestor(Object *a, Object *b)
{
    // array<ref<Object>>
    Object **o;
    Object *r;
    int n;

    if(paint(&a->hash, 1, &b->hash, 1, &o, &n, Lca) == ERROR_NEG1 || n == 0)
        return nil;
    r = ref(o[0]);
    free(o);
    return r;
}
/*e: function [[ancestor]] */

/*s: function [[lca]] */
errorneg1
lca(Eval *ev)
{
    Object *a, *b;
    // array<ref<Object>>
    Object **o;
    int n;

    if(ev->nstk < 2){
        werrstr("ancestor needs 2 objects");
        return ERROR_NEG1;
    }
    n = 0;
    b = pop(ev);
    a = pop(ev);
    paint(&a->hash, 1, &b->hash, 1, &o, &n, Lca);
    if(n == 0)
        return ERROR_NEG1;
    push(ev, *o);
    free(o);
    return OK_0;
}
/*e: function [[lca]] */

/*s: function [[parent]] */
static errorneg1
parent(Eval *ev)
{
    Object *o, *p;

    o = pop(ev);
    /*s: [[parent()]] sanity check [[o]] is a commit */
    if(o->type != GCommit){
        werrstr("not a commit: %H", o->hash);
        return ERROR_NEG1;
    }
    /*e: [[parent()]] sanity check [[o]] is a commit */
    /* Special case: first commit has no parent. */
    if(o->commit->nparent == 0)
        p = emptydir();
    else {
        p = readobject(o->commit->parent[0]);
        /*s: [[parent()]] sanity check [[p]] */
        if (p == nil){
           werrstr("no parent for %H", o->hash);
           return ERROR_NEG1;
        }
        /*e: [[parent()]] sanity check [[p]] */
    }
    push(ev, p);
    return OK_0;
}
/*e: function [[parent]] */

/*s: function [[range]] */
static errorneg1
range(Eval *ev)
{
    Object *a, *b;
    // array<ref<Object>>
    Object **o;
    int i, n;

    b = pop(ev);
    a = pop(ev);
    /*s: [[range()]] if [[b]] has [[Zhash]] */
    if(hasheq(&b->hash, &Zhash))
        b = &zcommit;
    /*e: [[range()]] if [[b]] has [[Zhash]] */
    /*s: [[range()]] if [[a]] has [[Zhash]] */
    if(hasheq(&a->hash, &Zhash))
        a = &zcommit;
    /*e: [[range()]] if [[a]] has [[Zhash]] */
    if(a->type != GCommit || b->type != GCommit){
        werrstr("non-commit object in range");
        return ERROR_NEG1;
    }

    if(paint(&b->hash, 1, &a->hash, 1, &o, &n, Range) == ERROR_NEG1)
        return ERROR_NEG1;
    for(i = 0; i < n; i++)
        push(ev, o[i]);
    free(o);
    return OK_0;
}
/*e: function [[range]] */

/*s: function [[matchpfx]] */
static errorneg1
matchpfx(Hash *h, char *ref)
{
    int i, c;
    Hash pfx;
    char *p;

    memset(&pfx, 0, sizeof(Hash));
    for(i = 0, p = ref; *p; p++, i++){
        if((c = charval(*p)) == -1)
            return ERROR_NEG1;
        pfx.h[i/2] |= c;
        if((i & 1) == 0)
            pfx.h[i/2] <<= 4;
    }
    return expandprefix(h, pfx, i*4);
}
/*e: function [[matchpfx]] */

/*s: function [[readref (git9/ref.c)]] */
errorneg1
readref(Hash *h, char *ref)
{
    char buf[256], s[256];
    errorneg1 r;
    fdt f;
    int n;
    /*s: [[readref()]](ref.c) other locals */
    static char *try[] = 
       {"", "refs/", "refs/heads/", "refs/remotes/", "refs/tags/", nil};
    char **pfx;
    /*e: [[readref()]](ref.c) other locals */

    r = hparse(h, ref);
    if(r != ERROR_NEG1)
        return r;
    // else
    if(strcmp(ref, "HEAD") == 0){
        snprint(buf, sizeof(buf), ".git/HEAD");
        f = open(buf, OREAD);
        if(f == ERROR_NEG1)
            return ERROR_NEG1;
        n = readn(f, s, sizeof(s) - 1);
        if(n == ERROR_NEG1)
            return ERROR_NEG1;
        // else
        s[n] = '\0';
        strip(s);
        r = hparse(h, s);
        goto found;
    }
    // else
    /*s: [[readref()]](ref.c) try [[.git/refs]] prefixes */
    for(pfx = try; *pfx; pfx++){
        snprint(buf, sizeof(buf), ".git/%s%s", *pfx, ref);
        f = open(buf, OREAD);
        if(f == ERROR_NEG1)
            continue;
        n = readn(f, s, sizeof(s) - 1);
        if(n == ERROR_NEG1)
            // not closing??
            continue;
        // else
        s[n] = '\0';
        strip(s);
        r = hparse(h, s);
        close(f);
        goto found;
    }
    /*e: [[readref()]](ref.c) try [[.git/refs]] prefixes */
    /*s: [[readref()]](ref.c) try other prefixes */
    if((r = matchpfx(h, ref)) != ERROR_NEG1)
        return r;
    /*e: [[readref()]](ref.c) try other prefixes */
    // else
    return ERROR_NEG1;

found:
    if(r == ERROR_NEG1 && strncmp(s, "ref: ", 5) == 0)
        // recurse!
        r = readref(h, s + 5);
    return r;
}
/*e: function [[readref (git9/ref.c)]] */

/*s: function [[evalpostfix]] */
errorneg1
evalpostfix(Eval *ev)
{
    char name[256];
    Hash h;
    Object *o;
    errorneg1 ret;

    eatspace(ev);
    if(!word(ev, name, sizeof(name))){
        werrstr("expected name in expression");
        return ERROR_NEG1;
    }
    ret = readref(&h, name);
    /*s: [[evalpostfix()]] sanity check [[ret]] invalid ref */
    if(ret == ERROR_NEG1){
        werrstr("invalid ref %s", name);
        return ret;
    }
    /*e: [[evalpostfix()]] sanity check [[ret]] invalid ref */
    /*s: [[evalpostfix()]] if [[Zhash]] */
    if(hasheq(&h, &Zhash))
        o = &zcommit;
    /*e: [[evalpostfix()]] if [[Zhash]] */
    else {
        o = readobject(h);
        /*s: [[evalpostfix()]] sanity check [[o]] invalid ref */
        if(o == nil){
          werrstr("invalid ref %s (hash %H)", name, h);
          return ERROR_NEG1;
        }
        /*e: [[evalpostfix()]] sanity check [[o]] invalid ref */
    }
    push(ev, o);

    while(true){
        eatspace(ev);
        switch(ev->p[0]){
        case '^':
        case '~':
            ev->p++;
            ret = parent(ev);
            /*s: [[evalpostfix()]] sanity check [[ret]] */
            if(ret == ERROR_NEG1)
                return ret;
            /*e: [[evalpostfix()]] sanity check [[ret]] */
            break;
        case '@':
            ev->p++;
            ret = lca(ev);
            /*s: [[evalpostfix()]] sanity check [[ret]] */
            if(ret == ERROR_NEG1)
                return ret;
            /*e: [[evalpostfix()]] sanity check [[ret]] */
            break;
        default:
            goto done;
            break;
        }   
    }
done:
    return OK_0;
}
/*e: function [[evalpostfix]] */

/*s: function [[evalexpr]] */
errorneg1
evalexpr(Eval *ev, char *ref)
{
    memset(ev, 0, sizeof(*ev));
    ev->str = ref;
    ev->p = ref;

    while(true){
        if(evalpostfix(ev) == ERROR_NEG1)
            return ERROR_NEG1;
        if(ev->p[0] == '\0')
            return OK_0;
        else if(take(ev, ":") || take(ev, "..")){
            if(evalpostfix(ev) == ERROR_NEG1)
                return ERROR_NEG1;
            if(ev->p[0] != '\0'){
                werrstr("junk at end of expression");
                return ERROR_NEG1;
            }
            return range(ev);
        }
    }
}
/*e: function [[evalexpr]] */

/*s: function [[resolverefs]] */
// main(get.c) -> <>
errorneg1
resolverefs(Hash **r, char *ref)
{
    Eval ev;
    Hash *h;
    int i;
    errorneg1 res;

    res = evalexpr(&ev, ref);

    if(res == ERROR_NEG1){
        free(ev.stk);
        return ERROR_NEG1;
    }
    h = eamalloc(ev.nstk, sizeof(Hash));
    for(i = 0; i < ev.nstk; i++)
        h[i] = ev.stk[i]->hash;
    *r = h;
    free(ev.stk);
    return ev.nstk;
}
/*e: function [[resolverefs]] */

/*s: function [[resolveref]] */
int
resolveref(Hash *r, char *ref)
{
    Eval ev;

    if(evalexpr(&ev, ref) == -1){
        free(ev.stk);
        return -1;
    }
    if(ev.nstk != 1){
        werrstr("ambiguous ref expr");
        free(ev.stk);
        return -1;
    }
    *r = ev.stk[0]->hash;
    free(ev.stk);
    return 0;
}
/*e: function [[resolveref]] */

/*s: function [[readrefdir]] */
int
readrefdir(Hash **refs, char ***names, int *nrefs, char *dpath, char *dname)
{
    Dir *d, *e, *dir;
    char *path, *name, *sep;
    int ndir;

    if((ndir = slurpdir(dpath, &dir)) == -1)
        return -1;
    sep = (*dname == '\0') ? "" : "/";
    e = dir + ndir;
    for(d = dir; d != e; d++){
        path = smprint("%s/%s", dpath, d->name);
        name = smprint("%s%s%s", dname, sep, d->name);
        if(d->mode & DMDIR) {
            if(readrefdir(refs, names, nrefs, path, name) == -1)
                goto noref;
        }else{
            *refs = erealloc(*refs, (*nrefs + 1)*sizeof(Hash));
            *names = erealloc(*names, (*nrefs + 1)*sizeof(char*));
            if(resolveref(&(*refs)[*nrefs], name) == -1)
                goto noref;
            (*names)[*nrefs] = name;
            *nrefs += 1;
            goto next;
        }
noref:      free(name);
next:       free(path);
    }
    free(dir);
    return 0;
}
/*e: function [[readrefdir]] */

/*s: function [[listrefs]] */
int
listrefs(Hash **refs, char ***names)
{
    int nrefs;

    *refs = nil;
    *names = nil;
    nrefs = 0;
    if(readrefdir(refs, names, &nrefs, ".git/refs", "") == -1){
        free(*refs);
        return -1;
    }
    return nrefs;
}
/*e: function [[listrefs]] */
/*e: git9/ref.c */
