/*s: cc/sub.c */
#include	"cc.h"

int	simpleg(long);
long	dotoffset(Type*, Type*, Node*);

extern	char	typechlvp[];
extern	char	typec[];
extern	char	typeh[];
extern	char	tab[NTYPE][NTYPE];

/*s: function new */
Node*
new(int t, Node *l, Node *r)
{
    Node *n;

    n = alloc(sizeof(Node));
    n->op = t;
    n->left = l;
    n->right = r;

    /*s: [[new()]] set lineno */
    if(l && t != OGOTO)
        n->lineno = l->lineno;
    else if(r)
        n->lineno = r->lineno;
    else
        n->lineno = lineno;
    /*x: [[new()]] set lineno */
    newflag = true;
    /*e: [[new()]] set lineno */

    return n;
}
/*e: function new */

/*s: function new1 */
Node*
new1(int o, Node *l, Node *r)
{
    Node *n;

    n = new(o, l, r);
    n->lineno = nearln;
    return n;
}
/*e: function new1 */

/*s: function prtree */
void
prtree(Node *n, char *s)
{

    print(" == %s ==\n", s);
    prtree1(n, 0, false);
    print("\n");
}
/*e: function prtree */

/*s: function prtree1 */
void
prtree1(Node *n, int d, bool f)
{
    //bitset<Left|Right> to decide whether to recurse left and right child
    int i;

    if(f)
      for(i=0; i<d; i++)
        print("   ");
    if(n == Z) {
        print("Z\n");
        return;
    }

    if(n->op == OLIST) {
        prtree1(n->left, d, false);
        prtree1(n->right, d, true);
        return;
    }
    d++;
    print("%O", n->op);
    i = 3; // 1 | 2

    switch(n->op) {
    case OCONST:
        if(typefd[n->type->etype])
            print(" \"%.8e\"", n->fconst);
        else
            print(" \"%lld\"", n->vconst);
        i = 0;
        break;

    case OSTRING:
        print(" \"%s\"", n->cstring);
        i = 0;
        break;
    case OLSTRING:
        if(sizeof(TRune) == sizeof(Rune))
            print(" \"%S\"", (Rune*)n->rstring);
        else
            print(" \"...\"");
        i = 0;
        break;

    case ONAME:
        print(" \"%F\"", n);
        print(" %ld", n->xoffset);
        i = 0;
        break;
    case ODOT:
    case OELEM:
        print(" \"%F\"", n);
        break;

    case OREGISTER:
        if(n->xoffset)
            print(" %ld+R%d", n->xoffset, n->reg);
        else
            print(" R%d", n->reg);
        i = 0;
        break;
    case OINDREG:
        print(" %ld(R%d)", n->xoffset, n->reg);
        i = 0;
        break;

    }

    if(n->type != T)
        print(" %T", n->type);

    if(n->addable != 0)
        print(" <%d>", n->addable);
    if(n->complex != 0)
        print(" (%d)", n->complex);

    print(" %L\n", n->lineno);

    if(i & 2)
        prtree1(n->left, d, true);
    if(i & 1)
        prtree1(n->right, d, true);
}
/*e: function prtree1 */

/*s: function typ */
Type*
typ(int et, Type *d)
{
    Type *t;

    t = alloc(sizeof(Type));
    t->etype = et;
    t->link = d;
    t->down = T;
    t->garb = GXXX;
    t->sym = S;

    t->width = ewidth[et];

    t->offset = 0;
    t->shift = 0;
    t->nbits = 0;

    return t;
}
/*e: function typ */

/*s: function copytyp */
Type*
copytyp(Type *t)
{
    Type *nt;

    nt = typ(TXXX, T); // could just do alloc(sizeof(Type))
    *nt = *t;
    return nt;
}
/*e: function copytyp */

/*s: function garbt */
//@Scheck: used by cc.y
Type* garbt(Type *t, long b)
{
    Type *t1;

    if(b & BGARB) {
        t1 = copytyp(t);
        t1->garb = simpleg(b);
        return t1;
    }
    return t;
}
/*e: function garbt */

/*s: function simpleg */
int
simpleg(long b)
{

    b &= BGARB;
    switch(b) {
    case BCONSTNT:           return GCONSTNT;
    case BVOLATILE:          return GVOLATILE;
    case BVOLATILE|BCONSTNT: return GCONSTNT|GVOLATILE;
    }
    return GXXX;
}
/*e: function simpleg */

/*s: function simplec */
//@Scheck: used by cc.y
int simplec(long b)
{

    b &= BCLASS;
    switch(b) {
    case 0: 
    // skipping register
    case BREGISTER:
        return CXXX;
    case BAUTO:
    case BAUTO|BREGISTER:
        return CAUTO;
    case BEXTERN:
        return CEXTERN;
    case BSTATIC:
        return CSTATIC;

    /*s: [[simplec()]] cases */
    case BTYPEDEF:
        return CTYPEDEF;
    /*x: [[simplec()]] cases */
    case BEXTERN|BREGISTER:
        return CEXREG;
    /*x: [[simplec()]] cases */
    case BTYPESTR:
        return CTYPESTR;
    /*e: [[simplec()]] cases */
    }
    diag(Z, "illegal combination of classes %Q", b);
    return CXXX;
}
/*e: function simplec */

/*s: function simplet */
Type*
simplet(long b)
{

    b &= ~BCLASS & ~BGARB;

    switch(b) {
    case BCHAR:
    case BCHAR|BSIGNED:
        return types[TCHAR];

    case BCHAR|BUNSIGNED:
        return types[TUCHAR];

    case BSHORT:
    case BSHORT|BINT:
    case BSHORT|BSIGNED:
    case BSHORT|BINT|BSIGNED:
        return types[TSHORT];

    case BUNSIGNED|BSHORT:
    case BUNSIGNED|BSHORT|BINT:
        return types[TUSHORT];

    case 0:
    case BINT:
    case BINT|BSIGNED:
    case BSIGNED:
        return types[TINT];

    case BUNSIGNED:
    case BUNSIGNED|BINT:
        return types[TUINT];

    case BLONG:
    case BLONG|BINT:
    case BLONG|BSIGNED:
    case BLONG|BINT|BSIGNED:
        return types[TLONG];

    case BUNSIGNED|BLONG:
    case BUNSIGNED|BLONG|BINT:
        return types[TULONG];

    case BVLONG|BLONG:
    case BVLONG|BLONG|BINT:
    case BVLONG|BLONG|BSIGNED:
    case BVLONG|BLONG|BINT|BSIGNED:
        return types[TVLONG];

    case BVLONG|BLONG|BUNSIGNED:
    case BVLONG|BLONG|BINT|BUNSIGNED:
        return types[TUVLONG];

    case BFLOAT:
        return types[TFLOAT];

    case BDOUBLE:
    case BDOUBLE|BLONG:
    case BFLOAT|BLONG:
        return types[TDOUBLE];

    case BVOID:
        return types[TVOID];
    }

    diag(Z, "illegal combination of types %Q", b);
    return types[TINT];
}
/*e: function simplet */

/*s: function stcompat */
bool
stcompat(Node *n, Type *t1, Type *t2, long ttab[])
{
    int i;
    ulong b;

    i = 0;
    if(t2 != T)
        i = t2->etype;
    b = 1L << i;
    i = 0;
    if(t1 != T)
        i = t1->etype;
    if(b & ttab[i]) {
        if(ttab == tasign)
            if(b == BSTRUCT || b == BUNION)
                if(!sametype(t1, t2))
                    return true;
        if(n->op != OCAST)
          if(b == BIND && i == TIND)
                if(!sametype(t1, t2))
                    return true;
        return false;
    }
    return true;
}
/*e: function stcompat */

/*s: function tcompat */
bool
tcompat(Node *n, Type *t1, Type *t2, long ttab[])
{

    if(stcompat(n, t1, t2, ttab)) {
        if(t1 == T)
            diag(n, "incompatible type: \"%T\" for op \"%O\"",
                t2, n->op);
        else
            diag(n, "incompatible types: \"%T\" and \"%T\" for op \"%O\"",
                t1, t2, n->op);
        return true;
    }
    return false;
}
/*e: function tcompat */

/*s: function makedot */
void
makedot(Node *n, Type *t, long o)
{
    Node *n1, *n2;

    if(t->nbits) {
        n1 = new(OXXX, Z, Z);
        *n1 = *n;
        n->op = OBIT;
        n->left = n1;
        n->right = Z;
        n->type = t;
        n->addable = n1->left->addable;
        n = n1;
    }
    n->addable = n->left->addable;
    if(n->addable == 0) {
        n1 = new1(OCONST, Z, Z);
        n1->vconst = o;
        n1->type = types[TLONG];
        n->right = n1;
        n->type = t;
        return;
    }
    n->left->type = t;
    if(o == 0) {
        *n = *n->left;
        return;
    }
    n->type = t;
    n1 = new1(OCONST, Z, Z);
    n1->vconst = o;
    t = typ(TIND, t);
    t->width = types[TIND]->width;
    n1->type = t;

    n2 = new1(OADDR, n->left, Z);
    n2->type = t;

    n1 = new1(OADD, n1, n2);
    n1->type = t;

    n->op = OIND;
    n->left = n1;
    n->right = Z;
}
/*e: function makedot */

/*s: function dotsearch */
Type*
dotsearch(Sym *s, Type *t, Node *n, long *off)
{
    Type *t1, *xt, *rt;

    xt = T;

    /*
     * look it up by name
     */
    for(t1 = t; t1 != T; t1 = t1->down)
        if(t1->sym == s) {
            if(xt != T)
                goto ambig;
            xt = t1;
        }

    /*
     * look it up by type
     */
    if(s->class == CTYPEDEF || s->class == CTYPESTR)
        for(t1 = t; t1 != T; t1 = t1->down)
            if(t1->sym == S && typesu[t1->etype])
                if(sametype(s->type, t1)) {
                    if(xt != T)
                        goto ambig;
                    xt = t1;
                }
    if(xt != T) {
        *off = xt->offset;
        return xt;
    }

    /*
     * look it up in unnamed substructures
     */
    for(t1 = t; t1 != T; t1 = t1->down)
        if(t1->sym == S && typesu[t1->etype]){
            rt = dotsearch(s, t1->link, n, off);
            if(rt != T) {
                if(xt != T)
                    goto ambig;
                xt = rt;
                *off += t1->offset;
            }
        }
    return xt;

ambig:
    diag(n, "ambiguous structure element: %s", s->name);
    return xt;
}
/*e: function dotsearch */

/*s: function dotoffset */
long
dotoffset(Type *st, Type *lt, Node *n)
{
    Type *t;
    Sym *g;
    long o, o1;

    o = -1;
    /*
     * first try matching at the top level
     * for matching tag names
     */
    g = st->tag;
    if(g != S)
        for(t=lt->link; t!=T; t=t->down)
            if(t->sym == S)
                if(g == t->tag) {
                    if(o >= 0)
                        goto ambig;
                    o = t->offset;
                }
    if(o >= 0)
        return o;

    /*
     * second try matching at the top level
     * for similar types
     */
    for(t=lt->link; t!=T; t=t->down)
        if(t->sym == S)
            if(sametype(st, t)) {
                if(o >= 0)
                    goto ambig;
                o = t->offset;
            }
    if(o >= 0)
        return o;

    /*
     * last try matching sub-levels
     */
    for(t=lt->link; t!=T; t=t->down)
        if(t->sym == S)
        if(typesu[t->etype]) {
            o1 = dotoffset(st, t, n);
            if(o1 >= 0) {
                if(o >= 0)
                    goto ambig;
                o = o1 + t->offset;
            }
        }
    return o;

ambig:
    diag(n, "ambiguous unnamed structure element");
    return o;
}
/*e: function dotoffset */

/*s: function allfloat */
/*
 * look into tree for floating point constant expressions
 */
bool
allfloat(Node *n, bool flag)
{

    if(n != Z) {
        if(n->type->etype != TDOUBLE)
            return true;
        switch(n->op) {
        case OCONST:
            if(flag)
                n->type = types[TFLOAT];
            return true;
        case OADD:	/* no need to get more exotic than this */
        case OSUB:
        case OMUL:
        case ODIV:
            if(!allfloat(n->right, flag))
                break;
        case OCAST:
            if(!allfloat(n->left, flag))
                break;
            if(flag)
                n->type = types[TFLOAT];
            return true;
        }
    }
    return false;
}
/*e: function allfloat */

/*s: function constas */
void
constas(Node *n, Type *il, Type *ir)
{
    Type *l, *r;

    l = il;
    r = ir;

    if(l == T)
        return;
    if(l->garb & GCONSTNT) {
        warn(n, "assignment to a constant type (%T)", il);
        return;
    }
    if(r == T)
        return;
    for(;;) {
        if(l->etype != TIND || r->etype != TIND)
            break;
        l = l->link;
        r = r->link;
        if(l == T || r == T)
            break;
        if(r->garb & GCONSTNT)
            if(!(l->garb & GCONSTNT)) {
                warn(n, "assignment of a constant pointer type (%T)", ir);
                break;
            }
    }
}
/*e: function constas */

/*s: function typeext1 */
void
typeext1(Type *st, Node *l)
{
    if(st->etype == TFLOAT && allfloat(l, false))
        allfloat(l, true);
}
/*e: function typeext1 */

/*s: function typeext */
void
typeext(Type *st, Node *l)
{
    Type *lt;
    Node *n1, *n2;
    long o;

    lt = l->type;

    if(lt == T)
        return;

    if(st->etype == TIND && vconst(l) == 0) {
        l->type = st;
        l->vconst = 0;
        return;
    }
    /*s: [[typeext()]] float handling */
    typeext1(st, l);
    /*e: [[typeext()]] float handling */
    /*s: [[typeext()]] unnamed substructure handling */
    /*
     * extension of C
     * if assign of struct containing unnamed sub-struct
     * to type of sub-struct, insert the DOT.
     * if assign of *struct containing unnamed substruct
     * to type of *sub-struct, insert the add-offset
     */
    if(typesu[st->etype] && typesu[lt->etype]) {
        o = dotoffset(st, lt, l);
        if(o >= 0) {
            n1 = new1(OXXX, Z, Z);
            *n1 = *l;
            l->op = ODOT;
            l->left = n1;
            l->right = Z;
            makedot(l, st, o);
        }
        return;
    }

    if(st->etype == TIND && typesu[st->link->etype])
    if(lt->etype == TIND && typesu[lt->link->etype]) {
        o = dotoffset(st->link, lt->link, l);
        if(o >= 0) {
            l->type = st;
            if(o == 0)
                return;
            n1 = new1(OXXX, Z, Z);
            *n1 = *l;
            n2 = new1(OCONST, Z, Z);
            n2->vconst = o;
            n2->type = st;
            l->op = OADD;
            l->left = n1;
            l->right = n2;
        }
        return;
    }
    /*e: [[typeext()]] unnamed substructure handling */
}
/*e: function typeext */

/*s: function nocast */
/*
 * a cast that generates no code
 * (same size move)
 */
bool
nocast(Type *t1, Type *t2)
{
    int i, b;

    if(t1->nbits)
        return false;
    i = 0;
    if(t2 != T)
        i = t2->etype;
    b = 1<<i;
    i = 0;
    if(t1 != T)
        i = t1->etype;
    if(b & ncast[i])
        return true;
    return false;
}
/*e: function nocast */

/*s: function nilcast */
/*
 * a cast that has a noop semantic
 * (small to large, convert)
 */
bool
nilcast(Type *t1, Type *t2)
{
    int et1, et2;

    if(t1 == T)
        return false;
    if(t1->nbits)
        return false;
    if(t2 == T)
        return false;
    et1 = t1->etype;
    et2 = t2->etype;
    if(et1 == et2)
        return true;
    if(typefd[et1] && typefd[et2]) {
        if(ewidth[et1] < ewidth[et2])
            return true;
        return false;
    }
    if(typechlp[et1] && typechlp[et2]) {
        if(ewidth[et1] < ewidth[et2])
            return true;
        return false;
    }
    return false;
}
/*e: function nilcast */

/*s: function arith */
/*
 * "the usual arithmetic conversions are performed"
 */
void
arith(Node *n, int f)
{
    Type *t1, *t2;
    int i, j, k;
    Node *n1;
    long w;

    t1 = n->left->type;
    if(n->right == Z)
        t2 = t1;
    else
        t2 = n->right->type;
    i = TXXX;
    if(t1 != T)
        i = t1->etype;
    j = TXXX;
    if(t2 != T)
        j = t2->etype;

    k = tab[i][j];

    if(k == TIND) {
        if(i == TIND)
            n->type = t1;
        else
        if(j == TIND)
            n->type = t2;
    } else {
        /* convert up to at least int */
        if(f == 1)
        while(k < TINT)
            k += 2;
        n->type = types[k];
    }
    if(n->op == OSUB)
    if(i == TIND && j == TIND) {
        w = n->right->type->link->width;
        if(w < 1 || n->left->type->link == T || n->left->type->link->width < 1)
            goto bad;
        n->type = types[ewidth[TIND] <= ewidth[TLONG]? TLONG: TVLONG];
        if(1 && ewidth[TIND] > ewidth[TLONG]){
            n1 = new1(OXXX, Z, Z);
            *n1 = *n;
            n->op = OCAST;
            n->left = n1;
            n->right = Z;
            n->type = types[TLONG];
        }
        if(w > 1) {
            n1 = new1(OXXX, Z, Z);
            *n1 = *n;
            n->op = ODIV;
            n->left = n1;
            n1 = new1(OCONST, Z, Z);
            n1->vconst = w;
            n1->type = n->type;
            n->right = n1;
            w = vlog(n1);
            if(w >= 0) {
                n->op = OASHR;
                n1->vconst = w;
            }
        }
        return;
    }
    if(!sametype(n->type, n->left->type)) {
        n->left = new1(OCAST, n->left, Z);
        n->left->type = n->type;
        if(n->type->etype == TIND) {
            w = n->type->link->width;
            if(w < 1) {
                snap(n->type->link);
                w = n->type->link->width;
                if(w < 1)
                    goto bad;
            }
            if(w > 1) {
                n1 = new1(OCONST, Z, Z);
                n1->vconst = w;
                n1->type = n->type;
                n->left = new1(OMUL, n->left, n1);
                n->left->type = n->type;
            }
        }
    }
    if(n->right != Z)
    if(!sametype(n->type, n->right->type)) {
        n->right = new1(OCAST, n->right, Z);
        n->right->type = n->type;
        if(n->type->etype == TIND) {
            w = n->type->link->width;
            if(w < 1) {
                snap(n->type->link);
                w = n->type->link->width;
                if(w < 1)
                    goto bad;
            }
            if(w != 1) {
                n1 = new1(OCONST, Z, Z);
                n1->vconst = w;
                n1->type = n->type;
                n->right = new1(OMUL, n->right, n1);
                n->right->type = n->type;
            }
        }
    }
    return;
bad:
    diag(n, "pointer addition not fully declared: %T", n->type->link);
}
/*e: function arith */

/*s: function simplifyshift */
/*
 * try to rewrite shift & mask
 */
void
simplifyshift(Node *n)
{
    ulong c3;
    int o, s1, s2, c1, c2;

    if(!typechlp[n->type->etype])
        return;
    switch(n->op) {
    default:
        return;
    case OASHL:
        s1 = 0;
        break;
    case OLSHR:
        s1 = 1;
        break;
    case OASHR:
        s1 = 2;
        break;
    }
    if(n->right->op != OCONST)
        return;
    if(n->left->op != OAND)
        return;
    if(n->left->right->op != OCONST)
        return;
    switch(n->left->left->op) {
    default:
        return;
    case OASHL:
        s2 = 0;
        break;
    case OLSHR:
        s2 = 1;
        break;
    case OASHR:
        s2 = 2;
        break;
    }
    if(n->left->left->right->op != OCONST)
        return;

    c1 = n->right->vconst;
    c2 = n->left->left->right->vconst;
    c3 = n->left->right->vconst;

/*
    if(debug['h'])
        print("%.3o %ld %ld %d #%.lux\n",
            (s1<<3)|s2, c1, c2, topbit(c3), c3);
*/

    o = n->op;
    switch((s1<<3)|s2) {
    case 000:	/* (((e <<u c2) & c3) <<u c1) */
        c3 >>= c2;
        c1 += c2;
        if(c1 >= 32)
            break;
        goto rewrite1;

    case 002:	/* (((e >>s c2) & c3) <<u c1) */
        if(topbit(c3) >= (32-c2))
            break;
    case 001:	/* (((e >>u c2) & c3) <<u c1) */
        if(c1 > c2) {
            c3 <<= c2;
            c1 -= c2;
            o = OASHL;
            goto rewrite1;
        }
        c3 <<= c1;
        if(c1 == c2)
            goto rewrite0;
        c1 = c2-c1;
        o = OLSHR;
        goto rewrite2;

    case 022:	/* (((e >>s c2) & c3) >>s c1) */
        if(c2 <= 0)
            break;
    case 012:	/* (((e >>s c2) & c3) >>u c1) */
        if(topbit(c3) >= (32-c2))
            break;
        goto s11;
    case 021:	/* (((e >>u c2) & c3) >>s c1) */
        if(topbit(c3) >= 31 && c2 <= 0)
            break;
        goto s11;
    case 011:	/* (((e >>u c2) & c3) >>u c1) */
    s11:
        c3 <<= c2;
        c1 += c2;
        if(c1 >= 32)
            break;
        o = OLSHR;
        goto rewrite1;

    case 020:	/* (((e <<u c2) & c3) >>s c1) */
        if(topbit(c3) >= 31)
            break;
    case 010:	/* (((e <<u c2) & c3) >>u c1) */
        c3 >>= c1;
        if(c1 == c2)
            goto rewrite0;
        if(c1 > c2) {
            c1 -= c2;
            goto rewrite2;
        }
        c1 = c2 - c1;
        o = OASHL;
        goto rewrite2;
    }
    return;

rewrite0:	/* get rid of both shifts */
if(debug['<'])prtree(n, "rewrite0");
    *n = *n->left;
    n->left = n->left->left;
    n->right->vconst = c3;
    return;
rewrite1:	/* get rid of lower shift */
if(debug['<'])prtree(n, "rewrite1");
    n->left->left = n->left->left->left;
    n->left->right->vconst = c3;
    n->right->vconst = c1;
    n->op = o;
    return;
rewrite2:	/* get rid of upper shift */
if(debug['<'])prtree(n, "rewrite2");
    *n = *n->left;
    n->right->vconst = c3;
    n->left->right->vconst = c1;
    n->left->op = o;
}
/*e: function simplifyshift */

/*s: function side */
bool
side(Node *n)
{

loop:
    if(n != Z)
    switch(n->op) {
    case OCAST:
    case ONOT:
    case OADDR:
    case OIND:
        n = n->left;
        goto loop;

    case OCOND:
        if(side(n->left))
            break;
        n = n->right;

    case OEQ:
    case ONE:
    case OLT:
    case OGE:
    case OGT:
    case OLE:
    case OADD:
    case OSUB:
    case OMUL:
    case OLMUL:
    case ODIV:
    case OLDIV:
    case OLSHR:
    case OASHL:
    case OASHR:
    case OAND:
    case OOR:
    case OXOR:
    case OMOD:
    case OLMOD:
    case OANDAND:
    case OOROR:
    case OCOMMA:
    case ODOT:
        if(side(n->left))
            break;
        n = n->right;
        goto loop;

    case OSIGN:
    case OSIZE:
    case OCONST:
    case OSTRING:
    case OLSTRING:
    case ONAME:
        return false;
    }
    return true;
}
/*e: function side */

/*s: function vconst */
int
vconst(Node *n)
{
    int i;

    if(n == Z)
        goto no;
    if(n->op != OCONST)
        goto no;
    if(n->type == T)
        goto no;

    switch(n->type->etype)
    {
    case TFLOAT:
    case TDOUBLE:
        i = 100;
        if(n->fconst > i || n->fconst < -i)
            goto no;
        i = n->fconst;
        if(i != n->fconst)
            goto no;
        return i;

    case TVLONG:
    case TUVLONG:
        i = n->vconst;
        if(i != n->vconst)
            goto no;
        return i;

    case TCHAR:
    case TUCHAR:
    case TSHORT:
    case TUSHORT:
    case TINT:
    case TUINT:
    case TLONG:
    case TULONG:
    case TIND:
        i = n->vconst;
        if(i != n->vconst)
            goto no;
        return i;
    }
no:
    return -159;	/* first uninteresting constant */
}
/*e: function vconst */

/*s: function log2 */
/*
 * return log(n) if n is a power of 2 constant
 */
int
log2(uvlong v)
{
    int s, i;
    uvlong m;

    s = 0;
    m = MASK(8*sizeof(uvlong));
    for(i=32; i; i>>=1) {
        m >>= i;
        if(!(v & m)) {
            v >>= i;
            s += i;
        }
    }
    if(v == 1)
        return s;
    return -1;
}
/*e: function log2 */

/*s: function vlog */
int
vlog(Node *n)
{
    if(n->op != OCONST)
        goto bad;
    if(typefd[n->type->etype])
        goto bad;

    return log2(n->vconst);

bad:
    return -1;
}
/*e: function vlog */

/*s: function topbit */
int
topbit(ulong v)
{
    int i;

    for(i = -1; v; i++)
        v >>= 1;
    return i;
}
/*e: function topbit */

/*s: function relcon */
/*
 * try to cast a constant down
 * rather than cast a variable up
 * example:
 *	if(c == 'a')
 */
void
relcon(Node *l, Node *r)
{
    vlong v;

    if(l->op != OCONST)
        return;
    if(r->op != OCAST)
        return;
    if(!nilcast(r->left->type, r->type))
        return;
    switch(r->type->etype) {
    default:
        return;
    case TCHAR:
    case TUCHAR:
    case TSHORT:
    case TUSHORT:
        v = convvtox(l->vconst, r->type->etype);
        if(v != l->vconst)
            return;
        break;
    }
    l->type = r->left->type;
    *r = *r->left;
}
/*e: function relcon */

/*s: function relindex */
int
relindex(int o)
{

    switch(o) {
    case OEQ: return 0;
    case ONE: return 1;
    case OLE: return 2;
    case OLS: return 3;
    case OLT: return 4;
    case OLO: return 5;
    case OGE: return 6;
    case OHS: return 7;
    case OGT: return 8;
    case OHI: return 9;
    default:
        diag(Z, "bad in relindex: %O", o);
    }
}
/*e: function relindex */

/*s: function invert */
Node*
invert(Node *n)
{
    Node *i;

    if(n == Z || n->op != OLIST)
        return n;
    i = n;
    for(n = n->left; n != Z; n = n->left) {
        if(n->op != OLIST)
            break;
        i->left = n->right;
        n->right = i;
        i = n;
    }
    i->left = n;
    return i;
}
/*e: function invert */

/*s: function bitno */
int
bitno(long b)
{
    int i;

    for(i=0; i<32; i++)
        if(b & (1L<<i))
            return i;
    diag(Z, "bad in bitno");
    return 0;
}
/*e: function bitno */

/*s: function typebitor */
//@Scheck: used by cc.y
long typebitor(long a, long b)
{
    long c;

    c = a | b;
    if(a & b)
        if((a & b) == BLONG)
            c |= BVLONG;		/* long long => vlong */
        else
            warn(Z, "once is enough: %Q", a & b);
    return c;
}
/*e: function typebitor */

/*s: function diag */
void
diag(Node *n, char *fmt, ...)
{
    char buf[STRINGSZ];
    va_list arg;

    va_start(arg, fmt);
    vseprint(buf, buf+sizeof(buf), fmt, arg);
    va_end(arg);

    Bprint(&diagbuf, "%L %s\n", (n==Z)? nearln: n->lineno, buf);

    if(debug['X']){
        Bflush(&diagbuf);
        abort();
    }
    if(n != Z)
      if(debug['v'])
        prtree(n, "diagnostic");

    nerrors++;
    if(nerrors > 10) {
        Bprint(&diagbuf, "too many errors\n");
        errorexit();
    }
}
/*e: function diag */

/*s: function warn */
void
warn(Node *n, char *fmt, ...)
{
    char buf[STRINGSZ];
    va_list arg;

    if(debug['w'] || debug['W']) {
        va_start(arg, fmt);
        vseprint(buf, buf+sizeof(buf), fmt, arg);
        va_end(arg);

        if(debug['W']) {
            diag(n, "%s", buf);
            return;
        }

        Bprint(&diagbuf, "warning: %L %s\n", 
                 (n==Z) ? nearln : n->lineno, buf);

        if(n != Z)
          if(debug['v'])
            prtree(n, "warning");
    }
}
/*e: function warn */

/*s: function fatal */
void
fatal(Node *n, char *fmt, ...)
{
    char buf[STRINGSZ];
    va_list arg;

    va_start(arg, fmt);
    vseprint(buf, buf+sizeof(buf), fmt, arg);
    va_end(arg);
    Bprint(&diagbuf, "%L %s\n", 
              (n==Z)? nearln: n->lineno, buf);

    if(debug['X']){
        Bflush(&diagbuf);
        abort();
    }
    if(n != Z)
      if(debug['v'])
        prtree(n, "diagnostic");

    nerrors++;
    errorexit();
}
/*e: function fatal */

/*s: global thash1 */
ulong	thash1	= 0x2edab8c9;
/*e: global thash1 */
/*s: global thash2 */
ulong	thash2	= 0x1dc74fb8;
/*e: global thash2 */
/*s: global thash3 */
ulong	thash3	= 0x1f241331;
/*e: global thash3 */
/*s: global thash */
ulong	thash[NALLTYPES];
/*e: global thash */
/*s: global thashinit */
Init	thashinit[] =
{
    TXXX,		0x17527bbd,	0,

    TCHAR,		0x5cedd32b,	0,
    TUCHAR,		0x552c4454,	0,
    TSHORT,		0x63040b4b,	0,
    TUSHORT,	0x32a45878,	0,
    TINT,		0x4151d5bd,	0,
    TUINT,		0x5ae707d6,	0,
    TLONG,		0x5ef20f47,	0,
    TULONG,		0x36d8eb8f,	0,
    TVLONG,		0x6e5e9590,	0,
    TUVLONG,	0x75910105,	0,
    TFLOAT,		0x25fd7af1,	0,
    TDOUBLE,	0x7c40a1b2,	0,

    TIND,		0x1b832357,	0,
    TFUNC,		0x6babc9cb,	0,
    TARRAY,		0x7c50986d,	0,
    TVOID,		0x44112eff,	0,
    TSTRUCT,	0x7c2da3bf,	0,
    TUNION,		0x3eb25e98,	0,
    TENUM,		0x44b54f61,	0,

    TOLD,		0x22b15988,	0,
    TDOT,		0x0204f6b3,	0,

    -1,		0,		0,
};
/*e: global thashinit */

/*s: global bnames */
char*	bnames[NALIGN];
/*e: global bnames */
/*s: global bnamesinit */
Init	bnamesinit[] =
{
    Axxx,	0,	"Axxx",

    Ael1,	0,	"el1",
    Ael2,	0,	"el2",
    Asu2,	0,	"su2",
    Aarg0,	0,	"arg0",
    Aarg1,	0,	"arg1",
    Aarg2,	0,	"arg2",
    Aaut3,	0,	"aut3",
    -1,	0,	0,
};
/*e: global bnamesinit */

/*s: global tnames */
char*	tnames[NALLTYPES];
/*e: global tnames */
/*s: global tnamesinit */
Init	tnamesinit[] =
{
    TXXX,		0,	"TXXX",

    TCHAR,		0,	"CHAR",
    TUCHAR,		0,	"UCHAR",
    TSHORT,		0,	"SHORT",
    TUSHORT,	0,	"USHORT",
    TINT,		0,	"INT",
    TUINT,		0,	"UINT",
    TLONG,		0,	"LONG",
    TULONG,		0,	"ULONG",
    TVLONG,		0,	"VLONG",
    TUVLONG,	0,	"UVLONG",
    TFLOAT,		0,	"FLOAT",
    TDOUBLE,	0,	"DOUBLE",
    TIND,		0,	"IND",
    TFUNC,		0,	"FUNC",
    TARRAY,		0,	"ARRAY",
    TVOID,		0,	"VOID",
    TSTRUCT,	0,	"STRUCT",
    TUNION,		0,	"UNION",
    TENUM,		0,	"ENUM",
    TOLD,		0,	"OLD",
    TDOT,		0,	"DOT",

    -1,		0,	0,
};
/*e: global tnamesinit */

/*s: global gnames */
char*	gnames[NGTYPES];
/*e: global gnames */
/*s: global gnamesinit */
Init	gnamesinit[] =
{
    GXXX,		0,	"GXXX",
    GCONSTNT,		0,	"CONST",
    GVOLATILE,		0,	"VOLATILE",
    GVOLATILE|GCONSTNT,	0,	"CONST-VOLATILE",
    -1,			0,	0,
};
/*e: global gnamesinit */

/*s: global qnames */
char*	qnames[NALLTYPES];
/*e: global qnames */
/*s: global qnamesinit */
Init	qnamesinit[] =
{
    TXXX      ,		0,	"TXXX",

    TCHAR     ,		0,	"CHAR",
    TUCHAR    ,		0,	"UCHAR",
    TSHORT    ,		0,	"SHORT",
    TUSHORT   ,		0,	"USHORT",
    TINT      ,		0,	"INT",
    TUINT     ,		0,	"UINT",
    TLONG     ,		0,	"LONG",
    TULONG    ,		0,	"ULONG",
    TVLONG    ,		0,	"VLONG",
    TUVLONG   ,		0,	"UVLONG",
    TFLOAT    ,		0,	"FLOAT",
    TDOUBLE   ,		0,	"DOUBLE",
    TIND      ,		0,	"IND",
    TFUNC     ,		0,	"FUNC",
    TARRAY    ,		0,	"ARRAY",
    TVOID     ,		0,	"VOID",
    TSTRUCT   ,		0,	"STRUCT",
    TUNION    ,		0,	"UNION",
    TENUM     ,		0,	"ENUM",

    TAUTO     ,		0,	"AUTO",
    TEXTERN   ,		0,	"EXTERN",
    TSTATIC   ,		0,	"STATIC",
    TTYPEDEF  ,		0,	"TYPEDEF",
    TTYPESTR  ,		0,	"TYPESTR",
    TREGISTER ,		0,	"REGISTER",
    TCONSTNT  ,		0,	"CONSTNT",
    TVOLATILE ,		0,	"VOLATILE",
    TUNSIGNED ,		0,	"UNSIGNED",
    TSIGNED   ,		0,	"SIGNED",
    TDOT      ,		0,	"DOT",
    TOLD      ,		0,	"OLD",

    -1,		0,	0,
};
/*e: global qnamesinit */
/*s: global cnames */
char*	cnames[NCTYPES];
/*e: global cnames */
/*s: global cnamesinit */
Init	cnamesinit[] =
{
    CXXX     ,		0,	"CXXX",
    CAUTO    ,		0,	"AUTO",
    CEXTERN  ,		0,	"EXTERN",
    CGLOBL   ,		0,	"GLOBL",
    CSTATIC  ,		0,	"STATIC",
    CLOCAL   ,		0,	"LOCAL",
    CTYPEDEF ,		0,	"TYPEDEF",
    CTYPESTR ,		0,	"TYPESTR",
    CPARAM   ,		0,	"PARAM",
    CEXREG   ,		0,	"EXREG",
    -1,			0,	0,
};
/*e: global cnamesinit */

/*s: global onames */
char*	onames[OEND+1];
/*e: global onames */
/*s: global onamesinit */
Init	onamesinit[] =
{
    OXXX,		0,	"OXXX",
    OADD,		0,	"ADD",
    OADDR,		0,	"ADDR",
    OAND,		0,	"AND",
    OANDAND,	0,	"ANDAND",
    OARRAY,		0,	"ARRAY",
    OAS,		0,	"AS",
    OASI,		0,	"ASI",
    OASADD,		0,	"ASADD",
    OASAND,		0,	"ASAND",
    OASASHL,	0,	"ASASHL",
    OASASHR,	0,	"ASASHR",
    OASDIV,		0,	"ASDIV",
    OASHL,		0,	"ASHL",
    OASHR,		0,	"ASHR",
    OASLDIV,	0,	"ASLDIV",
    OASLMOD,	0,	"ASLMOD",
    OASLMUL,	0,	"ASLMUL",
    OASLSHR,	0,	"ASLSHR",
    OASMOD,		0,	"ASMOD",
    OASMUL,		0,	"ASMUL",
    OASOR,		0,	"ASOR",
    OASSUB,		0,	"ASSUB",
    OASXOR,		0,	"ASXOR",
    OBIT,		0,	"BIT",
    OBREAK,		0,	"BREAK",
    OCASE,		0,	"CASE",
    OCAST,		0,	"CAST",
    OCOMMA,		0,	"COMMA",
    OCOND,		0,	"COND",
    OCONST,		0,	"CONST",
    OCONTINUE,	0,	"CONTINUE",
    ODIV,		0,	"DIV",
    ODOT,		0,	"DOT",
    ODOTDOT,	0,	"DOTDOT",
    ODWHILE,	0,	"DWHILE",
    OEQ,		0,	"EQ",
    OFOR,		0,	"FOR",
    OFUNC,		0,	"FUNC",
    OGE,		0,	"GE",
    OGOTO,		0,	"GOTO",
    OGT,		0,	"GT",
    OHI,		0,	"HI",
    OHS,		0,	"HS",
    OIF,		0,	"IF",
    OIND,		0,	"IND",
    OINDREG,	0,	"INDREG",
    OINIT,		0,	"INIT",
    OLABEL,		0,	"LABEL",
    OLDIV,		0,	"LDIV",
    OLE,		0,	"LE",
    OLIST,		0,	"LIST",
    OLMOD,		0,	"LMOD",
    OLMUL,		0,	"LMUL",
    OLO,		0,	"LO",
    OLS,		0,	"LS",
    OLSHR,		0,	"LSHR",
    OLT,		0,	"LT",
    OMOD,		0,	"MOD",
    OMUL,		0,	"MUL",
    ONAME,		0,	"NAME",
    ONE,		0,	"NE",
    ONOT,		0,	"NOT",
    OOR,		0,	"OR",
    OOROR,		0,	"OROR",
    OPOSTDEC,	0,	"POSTDEC",
    OPOSTINC,	0,	"POSTINC",
    OPREDEC,	0,	"PREDEC",
    OPREINC,	0,	"PREINC",
    OPROTO,		0,	"PROTO",
    OREGISTER,	0,	"REGISTER",
    ORETURN,	0,	"RETURN",
    OSET,		0,	"SET",
    OSIGN,		0,	"SIGN",
    OSIZE,		0,	"SIZE",
    OSTRING,	0,	"STRING",
    OLSTRING,	0,	"LSTRING",
    OSTRUCT,	0,	"STRUCT",
    OSUB,		0,	"SUB",
    OSWITCH,	0,	"SWITCH",
    OUNION,		0,	"UNION",
    OUSED,		0,	"USED",
    OWHILE,		0,	"WHILE",
    OXOR,		0,	"XOR",
    OPOS,		0,	"POS",
    ONEG,		0,	"NEG",
    OCOM,		0,	"COM",
    OELEM,		0,	"ELEM",
    OINDEX,		0,	"INDEX",
    OREGPAIR,	0,	"REGPAIR",
    OEXREG,		0,	"EXREG",
    OEND,		0,	"END",
    -1,		0,	0,
};
/*e: global onamesinit */

/*s: global comrel */
/*	OEQ, ONE, OLE, OLS, OLT, OLO, OGE, OHS, OGT, OHI */
char	comrel[12] =
{
    ONE, OEQ, OGT, OHI, OGE, OHS, OLT, OLO, OLE, OLS,
};
/*e: global comrel */
/*s: global invrel */
char	invrel[12] =
{
    OEQ, ONE, OGE, OHS, OGT, OHI, OLE, OLS, OLT, OLO,
};
/*e: global invrel */
/*s: global logrel */
char	logrel[12] =
{
    OEQ, ONE, OLS, OLS, OLO, OLO, OHS, OHS, OHI, OHI,
};
/*e: global logrel */

/*s: global typei */
// set<type_kind>
char	typei[NTYPE];
/*e: global typei */
/*s: global typeiinit */
int	typeiinit[] =
{
    TCHAR, TUCHAR, TSHORT, TUSHORT, TINT, TUINT, TLONG, TULONG, TVLONG, TUVLONG, -1,
};
/*e: global typeiinit */
/*s: global typeu */
char	typeu[NTYPE];
/*e: global typeu */
/*s: global typeuinit */
int	typeuinit[] =
{
    TUCHAR, TUSHORT, TUINT, TULONG, TUVLONG, TIND, -1,
};
/*e: global typeuinit */

/*s: global typesuv */
char	typesuv[NTYPE];
/*e: global typesuv */
/*s: global typesuvinit */
int	typesuvinit[] =
{
    TVLONG, TUVLONG, TSTRUCT, TUNION, -1,
};
/*e: global typesuvinit */

/*s: global typeilp */
char	typeilp[NTYPE];
/*e: global typeilp */
/*s: global typeilpinit */
int	typeilpinit[] =
{
    TINT, TUINT, TLONG, TULONG, TIND, -1
};
/*e: global typeilpinit */

/*s: global typechl */
char	typechl[NTYPE];
/*e: global typechl */
/*s: global typechlv */
char	typechlv[NTYPE];
/*e: global typechlv */
/*s: global typechlvp */
char typechlvp[NTYPE];
/*e: global typechlvp */
/*s: global typechlinit */
int	typechlinit[] =
{
    TCHAR, TUCHAR, TSHORT, TUSHORT, TINT, TUINT, TLONG, TULONG, -1,
};
/*e: global typechlinit */

/*s: global typechlp */
char	typechlp[NTYPE];
/*e: global typechlp */
/*s: global typechlpinit */
int	typechlpinit[] =
{
    TCHAR, TUCHAR, TSHORT, TUSHORT, TINT, TUINT, TLONG, TULONG, TIND, -1,
};
/*e: global typechlpinit */

/*s: global typechlpfd */
char	typechlpfd[NTYPE];
/*e: global typechlpfd */
/*s: global typechlpfdinit */
int	typechlpfdinit[] =
{
    TCHAR, TUCHAR, TSHORT, TUSHORT, TINT, TUINT, TLONG, TULONG, TFLOAT, TDOUBLE, TIND, -1,
};
/*e: global typechlpfdinit */

/*s: global typec */
char	typec[NTYPE];
/*e: global typec */
/*s: global typecinit */
int	typecinit[] =
{
    TCHAR, TUCHAR, -1
};
/*e: global typecinit */

/*s: global typeh */
char	typeh[NTYPE];
/*e: global typeh */
/*s: global typehinit */
int	typehinit[] =
{
    TSHORT, TUSHORT, -1,
};
/*e: global typehinit */

/*s: global typeil */
char	typeil[NTYPE];
/*e: global typeil */
/*s: global typeilinit */
int	typeilinit[] =
{
    TINT, TUINT, TLONG, TULONG, -1,
};
/*e: global typeilinit */

/*s: global typev */
char	typev[NTYPE];
/*e: global typev */
/*s: global typevinit */
int	typevinit[] =
{
    TVLONG,	TUVLONG, -1,
};
/*e: global typevinit */

/*s: global typefd */
char	typefd[NTYPE];
/*e: global typefd */
/*s: global typefdinit */
int	typefdinit[] =
{
    TFLOAT, TDOUBLE, -1,
};
/*e: global typefdinit */

/*s: global typeaf */
char	typeaf[NTYPE];
/*e: global typeaf */
/*s: global typeafinit */
int	typeafinit[] =
{
    TFUNC, TARRAY, -1,
};
/*e: global typeafinit */

/*s: global typesu */
char	typesu[NTYPE];
/*e: global typesu */
/*s: global typesuinit */
int	typesuinit[] =
{
    TSTRUCT, TUNION, -1,
};
/*e: global typesuinit */

/*s: global tasign */
long	tasign[NTYPE];
/*e: global tasign */
/*s: global tasigninit */
Init	tasigninit[] =
{
    TCHAR,		BNUMBER,	0,
    TUCHAR,		BNUMBER,	0,
    TSHORT,		BNUMBER,	0,
    TUSHORT,	BNUMBER,	0,
    TINT,		BNUMBER,	0,
    TUINT,		BNUMBER,	0,
    TLONG,		BNUMBER,	0,
    TULONG,		BNUMBER,	0,
    TVLONG,		BNUMBER,	0,
    TUVLONG,	BNUMBER,	0,
    TFLOAT,		BNUMBER,	0,
    TDOUBLE,	BNUMBER,	0,
    TIND,		BIND,		0,
    TSTRUCT,	BSTRUCT,	0,
    TUNION,		BUNION,		0,
    -1,		0,		0,
};
/*e: global tasigninit */

/*s: global tasadd */
long	tasadd[NTYPE];
/*e: global tasadd */
/*s: global tasaddinit */
Init	tasaddinit[] =
{
    TCHAR,		BNUMBER,	0,
    TUCHAR,		BNUMBER,	0,
    TSHORT,		BNUMBER,	0,
    TUSHORT,	BNUMBER,	0,
    TINT,		BNUMBER,	0,
    TUINT,		BNUMBER,	0,
    TLONG,		BNUMBER,	0,
    TULONG,		BNUMBER,	0,
    TVLONG,		BNUMBER,	0,
    TUVLONG,	BNUMBER,	0,
    TFLOAT,		BNUMBER,	0,
    TDOUBLE,	BNUMBER,	0,
    TIND,		BINTEGER,	0,
    -1,		0,		0,
};
/*e: global tasaddinit */

/*s: global tcast */
long	tcast[NTYPE];
/*e: global tcast */
/*s: global tcastinit */
Init	tcastinit[] =
{
    TCHAR,		BNUMBER|BIND|BVOID,	0,
    TUCHAR,		BNUMBER|BIND|BVOID,	0,
    TSHORT,		BNUMBER|BIND|BVOID,	0,
    TUSHORT,	BNUMBER|BIND|BVOID,	0,
    TINT,		BNUMBER|BIND|BVOID,	0,
    TUINT,		BNUMBER|BIND|BVOID,	0,
    TLONG,		BNUMBER|BIND|BVOID,	0,
    TULONG,		BNUMBER|BIND|BVOID,	0,
    TVLONG,		BNUMBER|BIND|BVOID,	0,
    TUVLONG,	BNUMBER|BIND|BVOID,	0,
    TFLOAT,		BNUMBER|BVOID,		0,
    TDOUBLE,	BNUMBER|BVOID,		0,
    TIND,		BINTEGER|BIND|BVOID,	0,
    TVOID,		BVOID,			0,
    TSTRUCT,	BSTRUCT|BVOID,		0,
    TUNION,		BUNION|BVOID,		0,
    -1,		0,			0,
};
/*e: global tcastinit */

/*s: global tadd */
long	tadd[NTYPE];
/*e: global tadd */
/*s: global taddinit */
Init	taddinit[] =
{
    TCHAR,		BNUMBER|BIND,	0,
    TUCHAR,		BNUMBER|BIND,	0,
    TSHORT,		BNUMBER|BIND,	0,
    TUSHORT,	BNUMBER|BIND,	0,
    TINT,		BNUMBER|BIND,	0,
    TUINT,		BNUMBER|BIND,	0,
    TLONG,		BNUMBER|BIND,	0,
    TULONG,		BNUMBER|BIND,	0,
    TVLONG,		BNUMBER|BIND,	0,
    TUVLONG,	BNUMBER|BIND,	0,
    TFLOAT,		BNUMBER,	0,
    TDOUBLE,	BNUMBER,	0,
    TIND,		BINTEGER,	0,
    -1,		0,		0,
};
/*e: global taddinit */

/*s: global tsub */
long	tsub[NTYPE];
/*e: global tsub */
/*s: global tsubinit */
Init	tsubinit[] =
{
    TCHAR,		BNUMBER,	0,
    TUCHAR,		BNUMBER,	0,
    TSHORT,		BNUMBER,	0,
    TUSHORT,	BNUMBER,	0,
    TINT,		BNUMBER,	0,
    TUINT,		BNUMBER,	0,
    TLONG,		BNUMBER,	0,
    TULONG,		BNUMBER,	0,
    TVLONG,		BNUMBER,	0,
    TUVLONG,	BNUMBER,	0,
    TFLOAT,		BNUMBER,	0,
    TDOUBLE,	BNUMBER,	0,
    TIND,		BINTEGER|BIND,	0,
    -1,		0,		0,
};
/*e: global tsubinit */

/*s: global tmul */
long	tmul[NTYPE];
/*e: global tmul */
/*s: global tmulinit */
Init	tmulinit[] =
{
    TCHAR,		BNUMBER,	0,
    TUCHAR,		BNUMBER,	0,
    TSHORT,		BNUMBER,	0,
    TUSHORT,	BNUMBER,	0,
    TINT,		BNUMBER,	0,
    TUINT,		BNUMBER,	0,
    TLONG,		BNUMBER,	0,
    TULONG,		BNUMBER,	0,
    TVLONG,		BNUMBER,	0,
    TUVLONG,	BNUMBER,	0,
    TFLOAT,		BNUMBER,	0,
    TDOUBLE,	BNUMBER,	0,
    -1,		0,		0,
};
/*e: global tmulinit */

/*s: global tand */
long	tand[NTYPE];
/*e: global tand */
/*s: global tandinit */
Init	tandinit[] =
{
    TCHAR,		BINTEGER,	0,
    TUCHAR,		BINTEGER,	0,
    TSHORT,		BINTEGER,	0,
    TUSHORT,	BINTEGER,	0,
    TINT,		BNUMBER,	0,
    TUINT,		BNUMBER,	0,
    TLONG,		BINTEGER,	0,
    TULONG,		BINTEGER,	0,
    TVLONG,		BINTEGER,	0,
    TUVLONG,	BINTEGER,	0,
    -1,		0,		0,
};
/*e: global tandinit */

/*s: global trel */
long	trel[NTYPE];
/*e: global trel */
/*s: global trelinit */
Init	trelinit[] =
{
    TCHAR,		BNUMBER,	0,
    TUCHAR,		BNUMBER,	0,
    TSHORT,		BNUMBER,	0,
    TUSHORT,	BNUMBER,	0,
    TINT,		BNUMBER,	0,
    TUINT,		BNUMBER,	0,
    TLONG,		BNUMBER,	0,
    TULONG,		BNUMBER,	0,
    TVLONG,		BNUMBER,	0,
    TUVLONG,	BNUMBER,	0,
    TFLOAT,		BNUMBER,	0,
    TDOUBLE,	BNUMBER,	0,

    TIND,		BIND,		0,
    -1,		0,		0,
};
/*e: global trelinit */

/*s: global tfunct */
long	tfunct[1] =
{
    BFUNC,
};
/*e: global tfunct */

/*s: global tindir */
long	tindir[1] =
{
    BIND,
};
/*e: global tindir */

/*s: global tdot */
long	tdot[1] =
{
    BSTRUCT|BUNION,
};
/*e: global tdot */

/*s: global tnot */
long	tnot[1] =
{
    BNUMBER|BIND,
};
/*e: global tnot */

/*s: global targ */
long	targ[1] =
{
    BNUMBER|BIND|BSTRUCT|BUNION,
};
/*e: global targ */

/*s: global tab */
char	tab[NTYPE][NTYPE] =
{
  [TXXX] =	{ 0,
        },

  [TCHAR] =	{ 0,	TCHAR, TUCHAR, TSHORT, TUSHORT, TINT, TUINT, TLONG,
            TULONG, TVLONG, TUVLONG, TFLOAT, TDOUBLE, [TIND] = TIND,
        },
  [TUCHAR] =	{ 0,	TUCHAR, TUCHAR, TUSHORT, TUSHORT, TUINT, TUINT, TULONG,
            TULONG, TUVLONG, TUVLONG, TFLOAT, TDOUBLE, [TIND] = TIND,
        },
  [TSHORT] =	{ 0,	TSHORT, TUSHORT, TSHORT, TUSHORT, TINT, TUINT, TLONG,
            TULONG, TVLONG, TUVLONG, TFLOAT, TDOUBLE, [TIND] = TIND,
        },
  [TUSHORT] =	{ 0,	TUSHORT, TUSHORT, TUSHORT, TUSHORT, TUINT, TUINT, TULONG,
            TULONG, TUVLONG, TUVLONG, TFLOAT, TDOUBLE, [TIND] = TIND,
        },
  [TINT] =	{ 0,	TINT, TUINT, TINT, TUINT, TINT, TUINT, TLONG,
            TULONG, TVLONG, TUVLONG, TFLOAT, TDOUBLE, [TIND] = TIND,
        },
  [TUINT] =	{ 0,	TUINT, TUINT, TUINT, TUINT, TUINT, TUINT, TULONG,
            TULONG, TUVLONG, TUVLONG, TFLOAT, TDOUBLE, [TIND] = TIND,
        },
  [TLONG] =	{ 0,	TLONG, TULONG, TLONG, TULONG, TLONG, TULONG, TLONG,
            TULONG, TVLONG, TUVLONG, TFLOAT, TDOUBLE, [TIND] = TIND,
        },
  [TULONG] =	{ 0,	TULONG, TULONG, TULONG, TULONG, TULONG, TULONG, TULONG,
            TULONG, TUVLONG, TUVLONG, TFLOAT, TDOUBLE, [TIND] = TIND,
        },
  [TVLONG] =	{ 0,	TVLONG, TUVLONG, TVLONG, TUVLONG, TVLONG, TUVLONG, TVLONG,
            TUVLONG, TVLONG, TUVLONG, TFLOAT, TDOUBLE, [TIND] = TIND,
        },
  [TUVLONG] =	{ 0,	TUVLONG, TUVLONG, TUVLONG, TUVLONG, TUVLONG, TUVLONG, TUVLONG,
            TUVLONG, TUVLONG, TUVLONG, TFLOAT, TDOUBLE, [TIND] = TIND,
        },
  [TFLOAT] =	{ 0,	TFLOAT, TFLOAT, TFLOAT, TFLOAT, TFLOAT, TFLOAT, TFLOAT,
            TFLOAT, TFLOAT, TFLOAT, TFLOAT, TDOUBLE, [TIND] = TIND,
        },
  [TDOUBLE] =	{ 0,	TDOUBLE, TDOUBLE, TDOUBLE, TDOUBLE, TDOUBLE, TDOUBLE, TDOUBLE,
            TDOUBLE, TDOUBLE, TDOUBLE, TFLOAT, TDOUBLE, [TIND] = TIND,
        },
  [TIND] =	{ 0,	TIND, TIND, TIND, TIND, TIND, TIND, TIND,
             TIND, TIND, TIND, TIND, TIND, [TIND] = TIND,
        },
};
/*e: global tab */

/*s: function urk */
void
urk(char *name, int max, int i)
{
    if(i >= max) {
        fprint(2, "bad tinit: %s %d>=%d\n", name, i, max);
        exits("init");
    }
}
/*e: function urk */

/*s: function tinit */
void
tinit(void)
{
    int *ip;
    Init *p;

    /*s: [[tinit()]] initialise thash */
    for(p=thashinit; p->code >= 0; p++) {
        urk("thash", nelem(thash), p->code);
        thash[p->code] = p->value;
    }
    /*e: [[tinit()]] initialise thash */
    /*s: [[tinit()]] initialise xxxnames debugging arrays */
    for(p=bnamesinit; p->code >= 0; p++) {
        urk("bnames", nelem(bnames), p->code);
        bnames[p->code] = p->s;
    }
    for(p=tnamesinit; p->code >= 0; p++) {
        urk("tnames", nelem(tnames), p->code);
        tnames[p->code] = p->s;
    }
    for(p=gnamesinit; p->code >= 0; p++) {
        urk("gnames", nelem(gnames), p->code);
        gnames[p->code] = p->s;
    }
    for(p=qnamesinit; p->code >= 0; p++) {
        urk("qnames", nelem(qnames), p->code);
        qnames[p->code] = p->s;
    }
    for(p=cnamesinit; p->code >= 0; p++) {
        urk("cnames", nelem(cnames), p->code);
        cnames[p->code] = p->s;
    }
    for(p=onamesinit; p->code >= 0; p++) {
        urk("onames", nelem(onames), p->code);
        onames[p->code] = p->s;
    }
    /*e: [[tinit()]] initialise xxxnames debugging arrays */
    /*s: [[tinit()]] initialise typexxx type sets */
    for(ip=typeiinit; *ip>=0; ip++) {
        urk("typei", nelem(typei), *ip);
        typei[*ip] = 1;
    }
    for(ip=typeuinit; *ip>=0; ip++) {
        urk("typeu", nelem(typeu), *ip);
        typeu[*ip] = 1;
    }
    for(ip=typesuvinit; *ip>=0; ip++) {
        urk("typesuv", nelem(typesuv), *ip);
        typesuv[*ip] = true;
    }
    for(ip=typeilpinit; *ip>=0; ip++) {
        urk("typeilp", nelem(typeilp), *ip);
        typeilp[*ip] = 1;
    }
    for(ip=typechlinit; *ip>=0; ip++) {
        urk("typechl", nelem(typechl), *ip);
        typechl[*ip] = 1;
        typechlv[*ip] = 1;
        typechlvp[*ip] = 1;
    }
    for(ip=typechlpinit; *ip>=0; ip++) {
        urk("typechlp", nelem(typechlp), *ip);
        typechlp[*ip] = 1;
        typechlvp[*ip] = 1;
    }
    for(ip=typechlpfdinit; *ip>=0; ip++) {
        urk("typechlpfd", nelem(typechlpfd), *ip);
        typechlpfd[*ip] = 1;
    }
    for(ip=typecinit; *ip>=0; ip++) {
        urk("typec", nelem(typec), *ip);
        typec[*ip] = 1;
    }
    for(ip=typehinit; *ip>=0; ip++) {
        urk("typeh", nelem(typeh), *ip);
        typeh[*ip] = 1;
    }
    for(ip=typeilinit; *ip>=0; ip++) {
        urk("typeil", nelem(typeil), *ip);
        typeil[*ip] = 1;
    }
    for(ip=typevinit; *ip>=0; ip++) {
        urk("typev", nelem(typev), *ip);
        typev[*ip] = 1;
        typechlv[*ip] = 1;
        typechlvp[*ip] = 1;
    }
    for(ip=typefdinit; *ip>=0; ip++) {
        urk("typefd", nelem(typefd), *ip);
        typefd[*ip] = 1;
    }
    for(ip=typeafinit; *ip>=0; ip++) {
        urk("typeaf", nelem(typeaf), *ip);
        typeaf[*ip] = 1;
    }
    for(ip=typesuinit; *ip >= 0; ip++) {
        urk("typesu", nelem(typesu), *ip);
        typesu[*ip] = 1;
    }
    /*e: [[tinit()]] initialise typexxx type sets */
    /*s: [[tinit()]] initialise tcompat arrays */
    for(p=tasigninit; p->code >= 0; p++) {
        urk("tasign", nelem(tasign), p->code);
        tasign[p->code] = p->value;
    }
    for(p=tasaddinit; p->code >= 0; p++) {
        urk("tasadd", nelem(tasadd), p->code);
        tasadd[p->code] = p->value;
    }
    for(p=tcastinit; p->code >= 0; p++) {
        urk("tcast", nelem(tcast), p->code);
        tcast[p->code] = p->value;
    }
    for(p=taddinit; p->code >= 0; p++) {
        urk("tadd", nelem(tadd), p->code);
        tadd[p->code] = p->value;
    }
    for(p=tsubinit; p->code >= 0; p++) {
        urk("tsub", nelem(tsub), p->code);
        tsub[p->code] = p->value;
    }
    for(p=tmulinit; p->code >= 0; p++) {
        urk("tmul", nelem(tmul), p->code);
        tmul[p->code] = p->value;
    }
    for(p=tandinit; p->code >= 0; p++) {
        urk("tand", nelem(tand), p->code);
        tand[p->code] = p->value;
    }
    for(p=trelinit; p->code >= 0; p++) {
        urk("trel", nelem(trel), p->code);
        trel[p->code] = p->value;
    }
    /*e: [[tinit()]] initialise tcompat arrays */
    /*s: [[tinit()]] initialise 32 bits defaults type sets */
    typeswitch = typechl;
    /*x: [[tinit()]] initialise 32 bits defaults type sets */
    /* 32-bit defaults */
    typeword = typechlp;
    /*x: [[tinit()]] initialise 32 bits defaults type sets */
    typecmplx = typesuv;
    /*e: [[tinit()]] initialise 32 bits defaults type sets */
}
/*e: function tinit */

/*s: function deadhead */
/*
 * return true if it is impossible to jump into the middle of n.
 */
static bool
deadhead(Node *n, bool caseok)
{
loop:
    if(n == Z)
        return true;

    switch(n->op) {

    case OCASE:
        if(!caseok)
            return false;
        goto rloop;
    case OLABEL:
        return false;



    case OLIST:
        if(!deadhead(n->left, caseok))
            return false;
    rloop:
        n = n->right;
        goto loop;


    case OWHILE:
    case ODWHILE:
    case OFOR:
        goto rloop;


    case OIF:
        return deadhead(n->right->left, caseok) && 
               deadhead(n->right->right, caseok);

    case OSWITCH:
        return deadhead(n->right, true);


    case ORETURN:
    case OGOTO:
    case OCONTINUE:
    case OBREAK:
        return true;

    case OSET:
    case OUSED:
        return true;
    }
    return true;
}
/*e: function deadhead */

/*s: function deadheads */
bool
deadheads(Node *c)
{
    return deadhead(c->left, false) && deadhead(c->right, false);
}
/*e: function deadheads */

/*s: function mixedasop */
int
mixedasop(Type *l, Type *r)
{
    return !typefd[l->etype] && typefd[r->etype];
}
/*e: function mixedasop */
/*e: cc/sub.c */
