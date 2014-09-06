/*s: acid/dot.c */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <ctype.h>
#include <mach.h>
#include "acid.h"

/*s: function srch */
Type*
srch(Type *t, char *s)
{
    Type *f;

    f = 0;
    while(t) {
        if(strcmp(t->tag->name, s) == 0) {
            if(f == 0 || t->depth < f->depth)
                f = t;
        }
        t = t->next;
    }
    return f;
}
/*e: function srch */

/*s: function odot */
void
odot(Node *n, Node *r)
{
    char *s;
    Type *t;
    Node res;
    uvlong addr;

    s = n->sym->name;
    if(s == 0)
        fatal("dodot: no tag");

    expr(n->left, &res);
    if(res.comt == 0)
        error("no type specified for (expr).%s", s);

    if(res.type != TINT)
        error("pointer must be integer for (expr).%s", s);

    t = srch(res.comt, s);
    if(t == 0)
        error("no tag for (expr).%s", s);

    /* Propagate types */
    if(t->type) 
        r->comt = t->type->lt;
    
    addr = res.ival+t->offset;
    if(t->fmt == 'a') {
        r->op = OCONST;
        r->fmt = 'a';
        r->type = TINT;
        r->ival = addr;
    }
    else 
        indir(cormap, addr, t->fmt, r);

}
/*e: function odot */

/*s: global tail (acid/dot.c) */
static Type **tail;
/*e: global tail (acid/dot.c) */
/*s: global base */
static Lsym *base;
/*e: global base */

/*s: function buildtype */
void
buildtype(Node *m, int d)
{
    Type *t;

    if(m == ZN)
        return;

    switch(m->op) {
    case OLIST:
        buildtype(m->left, d);		
        buildtype(m->right, d);
        break;

    case OCTRUCT:
        buildtype(m->left, d+1);
        break;
    default:
        t = malloc(sizeof(Type));
        t->next = 0;
        t->depth = d;
        t->tag = m->sym;
        t->base = base;
        t->offset = m->ival;
        if(m->left) {
            t->type = m->left->sym;
            t->fmt = 'a';			
        }
        else {
            t->type = 0;
            if(m->right)
                t->type = m->right->sym;
            t->fmt = m->fmt;
        }

        *tail = t;
        tail = &t->next;
    }			
}
/*e: function buildtype */

/*s: function defcomplex */
void
defcomplex(Node *tn, Node *m)
{
    tail = &tn->sym->lt;
    base = tn->sym;
    buildtype(m, 0);
}
/*e: function defcomplex */

/*s: function decl */
void
decl(Node *n)
{
    Node *l;
    Value *v;
    Frtype *f;
    Lsym *type;

    type = n->sym;
    if(type->lt == 0)
        error("%s is not a complex type", type->name);

    l = n->left;
    if(l->op == ONAME) {
        v = l->sym->v;
        v->comt = type->lt;
        v->fmt = 'a';
        return;
    }

    /*
     * Frame declaration
     */
    for(f = l->sym->local; f; f = f->next) {
        if(f->var == l->left->sym) {
            f->type = n->sym->lt;
            return;
        }
    }
    f = malloc(sizeof(Frtype));
    if(f == 0)
        fatal("out of memory");

    f->type = type->lt;

    f->var = l->left->sym;
    f->next = l->sym->local;
    l->sym->local = f;
}
/*e: function decl */
/*e: acid/dot.c */
