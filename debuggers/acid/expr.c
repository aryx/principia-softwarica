/*s: acid/expr.c */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <ctype.h>
#include <mach.h>
#include "acid.h"

/*s: global fsize */
static int fsize[] =
{
    ['A'] 4,
    ['B'] 4,
    ['C'] 1,
    ['D'] 4,
    ['F'] 8,
    ['G'] 8,
    ['O'] 4,
    ['Q'] 4,
    ['R'] 4,
    ['S'] 4,
    ['U'] 4,
    ['V'] 8,
    ['W'] 8,
    ['X'] 4,
    ['Y'] 8,
    ['Z'] 8,
    ['a'] 4,
    ['b'] 1,
    ['c'] 1,
    ['d'] 2,
    ['f'] 4,
    ['g'] 4,
    ['o'] 2,
    ['q'] 2,
    ['r'] 2,
    ['s'] 4,
    ['u'] 2,
    ['x'] 2,
    ['3'] 10,
    ['8'] 10,
};
/*e: global fsize */

/*s: function fmtsize */
int
fmtsize(Value *v)
{
    int ret;

    switch(v->fmt) {
    default:
        return  fsize[v->fmt];
    case 'i':
    case 'I':
        if(v->type != TINT || machdata == 0)
            error("no size for i fmt pointer ++/--");
        ret = (*machdata->instsize)(cormap, v->ival);
        if(ret < 0) {
            ret = (*machdata->instsize)(symmap, v->ival);
            if(ret < 0)
                error("%r");
        }
        return ret;
    }
}
/*e: function fmtsize */

/*s: function chklval */
void
chklval(Node *lp)
{
    if(lp->op != ONAME)
        error("need l-value");
}
/*e: function chklval */

/*s: function olist */
void
olist(Node *n, Node *res)
{
    expr(n->left, res);
    expr(n->right, res);
}
/*e: function olist */

/*s: function oeval */
void
oeval(Node *n, Node *res)
{
    expr(n->left, res);
    if(res->type != TCODE)
        error("bad type for eval");
    expr(res->cc, res);
}
/*e: function oeval */

/*s: function ocast */
void
ocast(Node *n, Node *res)
{
    if(n->sym->lt == 0)
        error("%s is not a complex type", n->sym->name);

    expr(n->left, res);
    res->comt = n->sym->lt;
    res->fmt = 'a';
}
/*e: function ocast */

/*s: function oindm */
void
oindm(Node *n, Node *res)
{
    Map *m;
    Node l;

    m = cormap;
    if(m == 0)
        m = symmap;
    expr(n->left, &l);
    if(l.type != TINT)
        error("bad type for *");
    if(m == 0)
        error("no map for *");
    indir(m, l.ival, l.fmt, res);
    res->comt = l.comt;
}
/*e: function oindm */

/*s: function oindc */
void
oindc(Node *n, Node *res)
{
    Map *m;
    Node l;

    m = symmap;
    if(m == 0)
        m = cormap;
    expr(n->left, &l);
    if(l.type != TINT)
        error("bad type for @");
    if(m == 0)
        error("no map for @");
    indir(m, l.ival, l.fmt, res);
    res->comt = l.comt;
}
/*e: function oindc */

/*s: function oframe */
void
oframe(Node *n, Node *res)
{
    char *p;
    Node *lp;
    uvlong ival;
    Frtype *f;

    p = n->sym->name;
    while(*p && *p == '$')
        p++;
    lp = n->left;
    if(localaddr(cormap, p, lp->sym->name, &ival, rget) < 0)
        error("colon: %r");

    res->ival = ival;
    res->op = OCONST;
    res->fmt = 'X';
    res->type = TINT;

    /* Try and set comt */
    for(f = n->sym->local; f; f = f->next) {
        if(f->var == lp->sym) {
            res->comt = f->type;
            res->fmt = 'a';
            break;
        }
    }
}
/*e: function oframe */

/*s: function oindex */
void
oindex(Node *n, Node *res)
{
    Node l, r;

    expr(n->left, &l);
    expr(n->right, &r);

    if(r.type != TINT)
        error("bad type for []");

    switch(l.type) {
    default:
        error("lhs[] has bad type");
    case TINT:
        indir(cormap, l.ival+(r.ival*fsize[l.fmt]), l.fmt, res);
        res->comt = l.comt;
        res->fmt = l.fmt;
        break;
    case TLIST:
        nthelem(l.l, r.ival, res);
        break;
    case TSTRING:
        res->ival = 0;
        if(r.ival >= 0 && r.ival < l.string->len) {
            int xx8;	/* to get around bug in vc */
            xx8 = r.ival;
            res->ival = l.string->string[xx8];
        }
        res->op = OCONST;
        res->type = TINT;
        res->fmt = 'c';
        break;
    }
}
/*e: function oindex */

/*s: function oappend */
void
oappend(Node *n, Node *res)
{
    Value *v;
    Node r, l;
    int  empty;

    expr(n->left, &l);
    expr(n->right, &r);
    if(l.type != TLIST)
        error("must append to list");
    empty = (l.l == nil && (n->left->op == ONAME));
    append(res, &l, &r);
    if(empty) {
        v = n->left->sym->v;
        v->type = res->type;
        v->Store = res->Store;
        v->comt = res->comt;
    }
}
/*e: function oappend */

/*s: function odelete */
void
odelete(Node *n, Node *res)
{
    Node l, r;

    expr(n->left, &l);
    expr(n->right, &r);
    if(l.type != TLIST)
        error("must delete from list");
    if(r.type != TINT)
        error("delete index must be integer");

    delete(l.l, r.ival, res);
}
/*e: function odelete */

/*s: function ohead */
void
ohead(Node *n, Node *res)
{
    Node l;

    expr(n->left, &l);
    if(l.type != TLIST)
        error("head needs list");
    res->op = OCONST;
    if(l.l) {
        res->type = l.l->type;
        res->Store = l.l->Store;
    }
    else {
        res->type = TLIST;
        res->l = 0;
    }
}
/*e: function ohead */

/*s: function otail */
void
otail(Node *n, Node *res)
{
    Node l;

    expr(n->left, &l);
    if(l.type != TLIST)
        error("tail needs list");
    res->op = OCONST;
    res->type = TLIST;
    if(l.l)
        res->l = l.l->next;
    else
        res->l = 0;
}
/*e: function otail */

/*s: function oconst */
void
oconst(Node *n, Node *res)
{
    res->op = OCONST;
    res->type = n->type;
    res->Store = n->Store;
    res->comt = n->comt;
}
/*e: function oconst */

/*s: function oname */
void
oname(Node *n, Node *res)
{
    Value *v;

    v = n->sym->v;
    if(v->set == 0)
        error("%s used but not set", n->sym->name);
    res->op = OCONST;
    res->type = v->type;
    res->Store = v->Store;
    res->comt = v->comt;
}
/*e: function oname */

/*s: function octruct */
void
octruct(Node *n, Node *res)
{
    res->op = OCONST;
    res->type = TLIST;
    res->l = construct(n->left);
}
/*e: function octruct */

/*s: function oasgn */
void
oasgn(Node *n, Node *res)
{
    Node *lp, r;
    Value *v;

    lp = n->left;
    switch(lp->op) {
    case OINDM:
        windir(cormap, lp->left, n->right, res);
        break;
    case OINDC:
        windir(symmap, lp->left, n->right, res);
        break;
    default:
        chklval(lp);
        v = lp->sym->v;
        expr(n->right, &r);
        v->set = 1;
        v->type = r.type;
        v->Store = r.Store;
        res->op = OCONST;
        res->type = v->type;
        res->Store = v->Store;
        res->comt = v->comt;
    }
}
/*e: function oasgn */

/*s: function oadd */
void
oadd(Node *n, Node *res)
{
    Node l, r;

    if(n->right == nil){		/* unary + */
        expr(n->left, res);
        return;
    }
    expr(n->left, &l);
    expr(n->right, &r);
    res->fmt = l.fmt;
    res->op = OCONST;
    res->type = TFLOAT;
    switch(l.type) {
    default:
        error("bad lhs type +");
    case TINT:
        switch(r.type) {
        case TINT:
            res->type = TINT;
            res->ival = l.ival+r.ival;
            break;
        case TFLOAT:
            res->fval = l.ival+r.fval;
            break;
        default:
            error("bad rhs type +");
        }
        break;
    case TFLOAT:
        switch(r.type) {
        case TINT:
            res->fval = l.fval+r.ival;
            break;
        case TFLOAT:
            res->fval = l.fval+r.fval;
            break;
        default:
            error("bad rhs type +");
        }
        break;
    case TSTRING:
        if(r.type == TSTRING) {
            res->type = TSTRING;
            res->fmt = 's';
            res->string = stradd(l.string, r.string); 
            break;
        }
        if(r.type == TINT) {
            res->type = TSTRING;
            res->fmt = 's';
            res->string = straddrune(l.string, r.ival);
            break;
        }
        error("bad rhs for +");
    case TLIST:
        res->type = TLIST;
        switch(r.type) {
        case TLIST:
            res->l = addlist(l.l, r.l);
            break;
        default:
            r.left = 0;
            r.right = 0;
            res->l = addlist(l.l, construct(&r));
            break;
        }
    }
}
/*e: function oadd */

/*s: function osub */
void
osub(Node *n, Node *res)
{
    Node l, r;

    expr(n->left, &l);
    expr(n->right, &r);
    res->fmt = l.fmt;
    res->op = OCONST;
    res->type = TFLOAT;
    switch(l.type) {
    default:
        error("bad lhs type -");
    case TINT:
        switch(r.type) {
        case TINT:
            res->type = TINT;
            res->ival = l.ival-r.ival;
            break;
        case TFLOAT:
            res->fval = l.ival-r.fval;
            break;
        default:
            error("bad rhs type -");
        }
        break;
    case TFLOAT:
        switch(r.type) {
        case TINT:
            res->fval = l.fval-r.ival;
            break;
        case TFLOAT:
            res->fval = l.fval-r.fval;
            break;
        default:
            error("bad rhs type -");
        }
        break;
    }
}
/*e: function osub */

/*s: function omul */
void
omul(Node *n, Node *res)
{
    Node l, r;

    expr(n->left, &l);
    expr(n->right, &r);
    res->fmt = l.fmt;
    res->op = OCONST;
    res->type = TFLOAT;
    switch(l.type) {
    default:
        error("bad lhs type *");
    case TINT:
        switch(r.type) {
        case TINT:
            res->type = TINT;
            res->ival = l.ival*r.ival;
            break;
        case TFLOAT:
            res->fval = l.ival*r.fval;
            break;
        default:
            error("bad rhs type *");
        }
        break;
    case TFLOAT:
        switch(r.type) {
        case TINT:
            res->fval = l.fval*r.ival;
            break;
        case TFLOAT:
            res->fval = l.fval*r.fval;
            break;
        default:
            error("bad rhs type *");
        }
        break;
    }
}
/*e: function omul */

/*s: function odiv */
void
odiv(Node *n, Node *res)
{
    Node l, r;

    expr(n->left, &l);
    expr(n->right, &r);
    res->fmt = l.fmt;
    res->op = OCONST;
    res->type = TFLOAT;
    switch(l.type) {
    default:
        error("bad lhs type /");
    case TINT:
        switch(r.type) {
        case TINT:
            res->type = TINT;
            if(r.ival == 0)
                error("zero divide");
            res->ival = l.ival/r.ival;
            break;
        case TFLOAT:
            if(r.fval == 0)
                error("zero divide");
            res->fval = l.ival/r.fval;
            break;
        default:
            error("bad rhs type /");
        }
        break;
    case TFLOAT:
        switch(r.type) {
        case TINT:
            res->fval = l.fval/r.ival;
            break;
        case TFLOAT:
            res->fval = l.fval/r.fval;
            break;
        default:
            error("bad rhs type /");
        }
        break;
    }
}
/*e: function odiv */

/*s: function omod */
void
omod(Node *n, Node *res)
{
    Node l, r;

    expr(n->left, &l);
    expr(n->right, &r);
    res->fmt = l.fmt;
    res->op = OCONST;
    res->type = TINT;
    if(l.type != TINT || r.type != TINT)
        error("bad expr type %");
    res->ival = l.ival%r.ival;
}
/*e: function omod */

/*s: function olsh */
void
olsh(Node *n, Node *res)
{
    Node l, r;

    expr(n->left, &l);
    expr(n->right, &r);
    res->fmt = l.fmt;
    res->op = OCONST;
    res->type = TINT;
    if(l.type != TINT || r.type != TINT)
        error("bad expr type <<");
    res->ival = l.ival<<r.ival;
}
/*e: function olsh */

/*s: function orsh */
void
orsh(Node *n, Node *res)
{
    Node l, r;

    expr(n->left, &l);
    expr(n->right, &r);
    res->fmt = l.fmt;
    res->op = OCONST;
    res->type = TINT;
    if(l.type != TINT || r.type != TINT)
        error("bad expr type >>");
    res->ival = (uvlong)l.ival>>r.ival;
}
/*e: function orsh */

/*s: function olt */
void
olt(Node *n, Node *res)
{
    Node l, r;

    expr(n->left, &l);
    expr(n->right, &r);

    res->fmt = l.fmt;
    res->op = OCONST;
    res->type = TINT;
    switch(l.type) {
    default:
        error("bad lhs type <");
    case TINT:
        switch(r.type) {
        case TINT:
            res->ival = l.ival < r.ival;
            break;
        case TFLOAT:
            res->ival = l.ival < r.fval;
            break;
        default:
            error("bad rhs type <");
        }
        break;
    case TFLOAT:
        switch(r.type) {
        case TINT:
            res->ival = l.fval < r.ival;
            break;
        case TFLOAT:
            res->ival = l.fval < r.fval;
            break;
        default:
            error("bad rhs type <");
        }
        break;
    }
}
/*e: function olt */

/*s: function ogt */
void
ogt(Node *n, Node *res)
{
    Node l, r;

    expr(n->left, &l);
    expr(n->right, &r);
    res->fmt = 'D';
    res->op = OCONST;
    res->type = TINT;
    switch(l.type) {
    default:
        error("bad lhs type >");
    case TINT:
        switch(r.type) {
        case TINT:
            res->ival = l.ival > r.ival;
            break;
        case TFLOAT:
            res->ival = l.ival > r.fval;
            break;
        default:
            error("bad rhs type >");
        }
        break;
    case TFLOAT:
        switch(r.type) {
        case TINT:
            res->ival = l.fval > r.ival;
            break;
        case TFLOAT:
            res->ival = l.fval > r.fval;
            break;
        default:
            error("bad rhs type >");
        }
        break;
    }
}
/*e: function ogt */

/*s: function oleq */
void
oleq(Node *n, Node *res)
{
    Node l, r;

    expr(n->left, &l);
    expr(n->right, &r);
    res->fmt = 'D';
    res->op = OCONST;
    res->type = TINT;
    switch(l.type) {
    default:
        error("bad expr type <=");
    case TINT:
        switch(r.type) {
        case TINT:
            res->ival = l.ival <= r.ival;
            break;
        case TFLOAT:
            res->ival = l.ival <= r.fval;
            break;
        default:
            error("bad expr type <=");
        }
        break;
    case TFLOAT:
        switch(r.type) {
        case TINT:
            res->ival = l.fval <= r.ival;
            break;
        case TFLOAT:
            res->ival = l.fval <= r.fval;
            break;
        default:
            error("bad expr type <=");
        }
        break;
    }
}
/*e: function oleq */

/*s: function ogeq */
void
ogeq(Node *n, Node *res)
{
    Node l, r;

    expr(n->left, &l);
    expr(n->right, &r);
    res->fmt = 'D';
    res->op = OCONST;
    res->type = TINT;
    switch(l.type) {
    default:
        error("bad lhs type >=");
    case TINT:
        switch(r.type) {
        case TINT:
            res->ival = l.ival >= r.ival;
            break;
        case TFLOAT:
            res->ival = l.ival >= r.fval;
            break;
        default:
            error("bad rhs type >=");
        }
        break;
    case TFLOAT:
        switch(r.type) {
        case TINT:
            res->ival = l.fval >= r.ival;
            break;
        case TFLOAT:
            res->ival = l.fval >= r.fval;
            break;
        default:
            error("bad rhs type >=");
        }
        break;
    }
}
/*e: function ogeq */

/*s: function oeq */
void
oeq(Node *n, Node *res)
{
    Node l, r;

    expr(n->left, &l);
    expr(n->right, &r);
    res->fmt = 'D';
    res->op = OCONST;
    res->type = TINT;
    res->ival = 0;
    switch(l.type) {
    default:
        break;
    case TINT:
        switch(r.type) {
        case TINT:
            res->ival = l.ival == r.ival;
            break;
        case TFLOAT:
            res->ival = l.ival == r.fval;
            break;
        default:
            break;
        }
        break;
    case TFLOAT:
        switch(r.type) {
        case TINT:
            res->ival = l.fval == r.ival;
            break;
        case TFLOAT:
            res->ival = l.fval == r.fval;
            break;
        default:
            break;
        }
        break;
    case TSTRING:
        if(r.type == TSTRING) {
            res->ival = scmp(r.string, l.string);
            break;
        }
        break;
    case TLIST:
        if(r.type == TLIST) {
            res->ival = listcmp(l.l, r.l);
            break;
        }
        break;
    }
    if(n->op == ONEQ)
        res->ival = !res->ival;
}
/*e: function oeq */


/*s: function oland */
void
oland(Node *n, Node *res)
{
    Node l, r;

    expr(n->left, &l);
    expr(n->right, &r);
    res->fmt = l.fmt;
    res->op = OCONST;
    res->type = TINT;
    if(l.type != TINT || r.type != TINT)
        error("bad expr type &");
    res->ival = l.ival&r.ival;
}
/*e: function oland */

/*s: function oxor */
void
oxor(Node *n, Node *res)
{
    Node l, r;

    expr(n->left, &l);
    expr(n->right, &r);
    res->fmt = l.fmt;
    res->op = OCONST;
    res->type = TINT;
    if(l.type != TINT || r.type != TINT)
        error("bad expr type ^");
    res->ival = l.ival^r.ival;
}
/*e: function oxor */

/*s: function olor */
void
olor(Node *n, Node *res)
{
    Node l, r;

    expr(n->left, &l);
    expr(n->right, &r);
    res->fmt = l.fmt;
    res->op = OCONST;
    res->type = TINT;
    if(l.type != TINT || r.type != TINT)
        error("bad expr type |");
    res->ival = l.ival|r.ival;
}
/*e: function olor */

/*s: function ocand */
void
ocand(Node *n, Node *res)
{
    Node l, r;

    res->op = OCONST;
    res->type = TINT;
    res->ival = 0;
    res->fmt = 'D';
    expr(n->left, &l);
    if(fbool(&l) == 0)
        return;
    expr(n->right, &r);
    if(fbool(&r) == 0)
        return;
    res->ival = 1;
}
/*e: function ocand */

/*s: function onot */
void
onot(Node *n, Node *res)
{
    Node l;

    res->op = OCONST;
    res->type = TINT;
    res->ival = 0;
    expr(n->left, &l);
    res->fmt = l.fmt;
    if(fbool(&l) == 0)
        res->ival = 1;
}
/*e: function onot */

/*s: function ocor */
void
ocor(Node *n, Node *res)
{
    Node l, r;

    res->op = OCONST;
    res->type = TINT;
    res->ival = 0;
    res->fmt = 'D';
    expr(n->left, &l);
    if(fbool(&l)) {
        res->ival = 1;
        return;
    }
    expr(n->right, &r);
    if(fbool(&r)) {
        res->ival = 1;
        return;
    }
}
/*e: function ocor */

/*s: function oeinc */
void
oeinc(Node *n, Node *res)
{
    Value *v;

    chklval(n->left);
    v = n->left->sym->v;
    res->op = OCONST;
    res->type = v->type;
    switch(v->type) {
    case TINT:
        if(n->op == OEDEC)
            v->ival -= fmtsize(v);
        else
            v->ival += fmtsize(v);
        break;			
    case TFLOAT:
        if(n->op == OEDEC)
            v->fval--;
        else
            v->fval++;
        break;
    default:
        error("bad type for pre --/++");
    }
    res->Store = v->Store;
}
/*e: function oeinc */

/*s: function opinc */
void
opinc(Node *n, Node *res)
{
    Value *v;

    chklval(n->left);
    v = n->left->sym->v;
    res->op = OCONST;
    res->type = v->type;
    res->Store = v->Store;
    switch(v->type) {
    case TINT:
        if(n->op == OPDEC)
            v->ival -= fmtsize(v);
        else
            v->ival += fmtsize(v);
        break;			
    case TFLOAT:
        if(n->op == OPDEC)
            v->fval--;
        else
            v->fval++;
        break;
    default:
        error("bad type for post --/++");
    }
}
/*e: function opinc */

/*s: function ocall */
void
ocall(Node *n, Node *res)
{
    Lsym *s;
    Rplace *rsav;

    res->op = OCONST;		/* Default return value */
    res->type = TLIST;
    res->l = 0;

    chklval(n->left);
    s = n->left->sym;

    if(n->builtin && !s->builtin){
        error("no builtin %s", s->name);
        return;
    }
    if(s->builtin && (n->builtin || s->proc == 0)) {
        (*s->builtin)(res, n->right);
        return;
    }
    if(s->proc == 0)
        error("no function %s", s->name);

    rsav = ret;
    call(s->name, n->right, s->proc->left, s->proc->right, res);
    ret = rsav;
}
/*e: function ocall */

/*s: function ofmt */
void
ofmt(Node *n, Node *res)
{
    expr(n->left, res);
    res->fmt = n->right->ival;
}
/*e: function ofmt */

/*s: function owhat */
void
owhat(Node *n, Node *res)
{
    res->op = OCONST;		/* Default return value */
    res->type = TLIST;
    res->l = 0;
    whatis(n->sym);
}
/*e: function owhat */

/*s: global expop */
void (*expop[])(Node*, Node*) =
{
    [ONAME]		oname,
    [OCONST]	oconst,
    [OMUL]		omul,
    [ODIV]		odiv,
    [OMOD]		omod,
    [OADD]		oadd,
    [OSUB]		osub,
    [ORSH]		orsh,
    [OLSH]		olsh,
    [OLT]		olt,
    [OGT]		ogt,
    [OLEQ]		oleq,
    [OGEQ]		ogeq,
    [OEQ]		oeq,
    [ONEQ]		oeq,
    [OLAND]		oland,
    [OXOR]		oxor,
    [OLOR]		olor,
    [OCAND]		ocand,
    [OCOR]		ocor,
    [OASGN]		oasgn,
    [OINDM]		oindm,
    [OEDEC]		oeinc,
    [OEINC]		oeinc,
    [OPINC]		opinc,
    [OPDEC]		opinc,
    [ONOT]		onot,
    [OIF]		0,
    [ODO]		0,
    [OLIST]		olist,
    [OCALL]		ocall,
    [OCTRUCT]	octruct,
    [OWHILE]	0,
    [OELSE]		0,
    [OHEAD]		ohead,
    [OTAIL]		otail,
    [OAPPEND]	oappend,
    [ORET]		0,
    [OINDEX]	oindex,
    [OINDC]		oindc,
    [ODOT]		odot,
    [OLOCAL]	0,
    [OFRAME]	oframe,
    [OCOMPLEX]	0,
    [ODELETE]	odelete,
    [OCAST]		ocast,
    [OFMT]		ofmt,
    [OEVAL]		oeval,
    [OWHAT]		owhat,
};
/*e: global expop */
/*e: acid/expr.c */
