/*s: cc/dcl.c */
#include "cc.h"

Sym*	mkstatic(Sym*);
Type*	tcopy(Type*);
Node*	doinit(Sym*, Type*, long, Node*);
Node*	init1(Sym*, Type*, long, int);
Node*	newlist(Node*, Node*);
void	walkparam(Node*, int);
Type*	fnproto(Node*);
Type*	fnproto1(Node*);
int	anyproto(Node*);
void	dbgdecl(Sym*);
Decl*	push(void);
Decl*	push1(Sym*);
int	rsametype(Type*, Type*, int, int);
Type*	paramconv(Type*, int);
void	adecl(int, Type*, Sym*);
void	pdecl(int, Type*, Sym*);
void	tmerge(Type*, Sym*);

/*s: function dodecl */
Node*
dodecl(void (*f)(int,Type*,Sym*), int class, Type *t, Node *n)
{
    Sym *s;
    Node *n1;
    long v;

    nearln = lineno;
    lastfield = 0;

loop:
    if(n != Z)
     switch(n->op) {
     // can adjust t using information from node n
     /*s: [[dodecl()]] switch node kind cases */
     case ONAME:
         if(f == NODECL)
             break;
         s = n->sym;

         // callback! should set s->type
         (*f)(class, t, s);

         /*s: [[dodecl()]] case ONAME, if local static variable */
         if(s->class == CLOCAL)
             s = mkstatic(s);
         /*e: [[dodecl()]] case ONAME, if local static variable */
         firstbit = 0;

         n->sym = s;
         n->type = s->type;
         n->etype = (s->type != T) ? s->type->etype : TVOID;
         n->xoffset = s->offset;
         n->class = s->class;

         if(debug['d'])
             dbgdecl(s);
         acidvar(s);

         s->varlineno = lineno;
         break;
     /*x: [[dodecl()]] switch node kind cases */
     case OIND:
         t = typ(TIND, t);
         t->garb = n->garb;
         n = n->left;
         goto loop;
     /*x: [[dodecl()]] switch node kind cases */
     case OARRAY:
         t = typ(TARRAY, t);
         n1 = n->right; // possible size
         n = n->left;
         t->width = 0; // array[], size could be set by doinit (or specified in n1)
         /*s: [[dodecl()]] switch node kind cases, case OARRAY, if array has a size */
         if(n1 != Z) {
             complex(n1); // will call evconst()
             v = (n1->op == OCONST) ? n1->vconst : -1;
             if(v <= 0) {
                 diag(n, "array size must be a positive constant");
                 v = 1;
             }
             t->width = v * t->link->width;
         }
         /*e: [[dodecl()]] switch node kind cases, case OARRAY, if array has a size */
         goto loop;
     /*x: [[dodecl()]] switch node kind cases */
     case OFUNC:
         t = typ(TFUNC, t);
         t->down = fnproto(n);
         n = n->left;
         goto loop;
     /*x: [[dodecl()]] switch node kind cases */
     case OBIT:
         n1 = n->right;
         complex(n1);
         lastfield = -1;
         if(n1->op == OCONST)
             lastfield = n1->vconst;
         if(lastfield < 0) {
             diag(n, "field width must be non-negative constant");
             lastfield = 1;
         }
         if(lastfield == 0) {
             lastbit = 0;
             firstbit = 1;
             if(n->left != Z) {
                 diag(n, "zero width named field");
                 lastfield = 1;
             }
         }
         if(!typei[t->etype]) {
             diag(n, "field type must be int-like");
             t = types[TINT];
             lastfield = 1;
         }
         if(lastfield > tfield->width*8) {
             diag(n, "field width larger than field unit");
             lastfield = 1;
         }
         lastbit += lastfield;
         if(lastbit > tfield->width*8) {
             lastbit = lastfield;
             firstbit = 1;
         }
         n = n->left;
         goto loop;
     /*e: [[dodecl()]] switch node kind cases */
     default:
         diag(n, "unknown declarator: %O", n->op);
         break;
     }

    lastdcltype = t;
    return n;
}
/*e: function dodecl */

/*s: function mkstatic */
Sym*
mkstatic(Sym *s)
{
    Sym *s1;

    if(s->class != CLOCAL) // should never happen, use errorexit()?
        return s;

    snprint(symb, NSYMB, "%s$%d", s->name, s->block);
    s1 = lookup();

    if(s1->class != CSTATIC) {
        s1->type = s->type;
        s1->offset = s->offset;
        s1->block = s->block;
        s1->class = CSTATIC;
    }
    return s1;
}
/*e: function mkstatic */

/*s: function tcopy */
/*
 * make a copy of a typedef
 * the problem is to split out incomplete
 * arrays so that it is in the variable
 * rather than the typedef.
 */
Type*
tcopy(Type *t)
{
    Type *tl, *tx;
    int et;

    if(t == T)
        return t;
    et = t->etype;
    if(typesu[et])
        return t;

    tl = tcopy(t->link);
    if(tl != t->link || (et == TARRAY && t->width == 0)) {
        tx = copytyp(t);
        tx->link = tl;
        return tx;
    }
    return t;
}
/*e: function tcopy */

/*s: function doinit */
Node*
doinit(Sym *s, Type *t, long o, Node *a)
{
    Node *n;

    if(t == T)
        return Z;
    if(s->class == CEXTERN) {
        s->class = CGLOBL;
        if(debug['d'])
            dbgdecl(s);
    }
    if(debug['i']) {
        print("t = %T; o = %ld; n = %s\n", t, o, s->name);
        prtree(a, "doinit value");
    }


    n = initlist;
    if(a->op == OINIT)
        a = a->left;
    initlist = a;

    a = init1(s, t, o, 0);
    if(initlist != Z)
        diag(initlist, "more initializers than structure: %s",
            s->name);
    initlist = n;

    return a;
}
/*e: function doinit */

/*s: function peekinit */
/*
 * get next major operator,
 * dont advance initlist.
 */
Node*
peekinit(void)
{
    Node *a;

    a = initlist;

loop:
    if(a == Z)
        return a;
    if(a->op == OLIST) {
        a = a->left;
        goto loop;
    }
    return a;
}
/*e: function peekinit */

/*s: function nextinit */
/*
 * consume and return next element on
 * initlist. expand strings.
 */
Node*
nextinit(void)
{
    Node *a, *b, *n;

    a = initlist;
    n = Z;

    if(a == Z)
        return a;
    if(a->op == OLIST) {
        n = a->right;
        a = a->left;
    }
    if(a->op == OUSED) {
        a = a->left;
        b = new(OCONST, Z, Z);
        b->type = a->type->link;
        if(a->op == OSTRING) {
            b->vconst = convvtox(*a->cstring, TCHAR);
            a->cstring++;
        }
        if(a->op == OLSTRING) {
            b->vconst = convvtox(*a->rstring, TRUNE);
            a->rstring++;
        }
        a->type->width -= b->type->width;
        if(a->type->width <= 0)
            initlist = n;
        return b;
    }
    initlist = n;
    return a;
}
/*e: function nextinit */

/*s: function isstruct */
bool
isstruct(Node *a, Type *t)
{
    Node *n;

    switch(a->op) {
    case ODOTDOT:
        n = a->left;
        if(n && n->type && sametype(n->type, t))
            return true;
    case OSTRING: case OLSTRING:
    case OCONST:
    case OINIT:
    case OELEM:
        return false;
    }

    n = new(ODOTDOT, Z, Z);
    *n = *a;

    /*
     * ODOTDOT is a flag for tcom
     * a second tcom will not be performed
     */
    a->op = ODOTDOT;
    a->left = n;
    a->right = Z;

    if(tcom(n))
        return false;

    if(sametype(n->type, t))
        return true;
    return false;
}
/*e: function isstruct */

/*s: function init1 */
Node*
init1(Sym *s, Type *t, long o, int exflag)
{
    Node *a, *l, *r, nod;
    Type *t1;
    long e, w, so, mw;

    a = peekinit();
    if(a == Z)
        return Z;

    if(debug['i']) {
        print("t = %T; o = %ld; n = %s\n", t, o, s->name);
        prtree(a, "init1 value");
    }

    if(exflag && a->op == OINIT)
        return doinit(s, t, o, nextinit());

    switch(t->etype) {
    case TCHAR:
    case TUCHAR:
    case TINT:
    case TUINT:
    case TSHORT:
    case TUSHORT:
    case TLONG:
    case TULONG:
    case TVLONG:
    case TUVLONG:
    case TFLOAT:
    case TDOUBLE:

    case TIND:
    single:
        if(a->op == OARRAY || a->op == OELEM)
            return Z;

        a = nextinit();
        if(a == Z)
            return Z;

        if(t->nbits)
            diag(Z, "cannot initialize bitfields");
        if(s->class == CAUTO) {
            l = new(ONAME, Z, Z);
            l->sym = s;
            l->type = t;
            l->etype = TVOID;
            if(s->type)
                l->etype = s->type->etype;
            l->xoffset = s->offset + o;
            l->class = s->class;

            l = new(OASI, l, a);
            return l;
        }

        complex(a);
        if(a->type == T)
            return Z;

        if(a->op == OCONST) {
            if(vconst(a) && t->etype == TIND && a->type && a->type->etype != TIND){
                diag(a, "initialize pointer to an integer: %s", s->name);
                return Z;
            }
            if(!sametype(a->type, t)) {
                /* hoop jumping to save malloc */
                if(nodcast == Z)
                    nodcast = new(OCAST, Z, Z);
                nod = *nodcast;
                nod.left = a;
                nod.type = t;
                nod.lineno = a->lineno;
                complex(&nod);
                if(nod.type)
                    *a = nod;
            }
            if(a->op != OCONST) {
                diag(a, "initializer is not a constant: %s",
                    s->name);
                return Z;
            }
            if(vconst(a) == 0)
                return Z;
            goto gext;
        }
        if(t->etype == TIND) {
            while(a->op == OCAST) {
                warn(a, "CAST in initialization ignored");
                a = a->left;
            }
            if(!sametype(t, a->type)) {
                diag(a, "initialization of incompatible pointers: %s\n%T and %T",
                    s->name, t, a->type);
            }
            if(a->op == OADDR)
                a = a->left;
            goto gext;
        }

        while(a->op == OCAST)
            a = a->left;
        if(a->op == OADDR) {
            warn(a, "initialize pointer to an integer: %s", s->name);
            a = a->left;
            goto gext;
        }
        diag(a, "initializer is not a constant: %s", s->name);
        return Z;

    gext:
        gextern(s, a, o, t->width);

        return Z;

    case TARRAY:
        w = t->link->width;
        if(a->op == OSTRING || a->op == OLSTRING)
        if(typei[t->link->etype]) {
            /*
             * get rid of null if sizes match exactly
             */
            a = nextinit();
            mw = t->width/w;
            so = a->type->width/a->type->link->width;
            if(mw && so > mw) {
                if(so != mw+1)
                    diag(a, "string initialization larger than array");
                a->type->width -= a->type->link->width;
            }

            /*
             * arrange strings to be expanded
             * inside OINIT braces.
             */
            a = new(OUSED, a, Z);
            return doinit(s, t, o, a);
        }

        mw = -w;
        l = Z;
        for(e=0;;) {
            /*
             * peek ahead for element initializer
             */
            a = peekinit();
            if(a == Z)
                break;
            if(a->op == OELEM && t->link->etype != TSTRUCT)
                break;
            if(a->op == OARRAY) {
                if(e && exflag)
                    break;
                a = nextinit();
                r = a->left;
                complex(r);
                if(r->op != OCONST) {
                    diag(r, "initializer subscript must be constant");
                    return Z;
                }
                e = r->vconst;
                if(t->width != 0)
                    if(e < 0 || e*w >= t->width) {
                        diag(a, "initialization index out of range: %ld", e);
                        continue;
                    }
            }

            so = e*w;
            if(so > mw)
                mw = so;
            if(t->width != 0)
                if(mw >= t->width)
                    break;
            r = init1(s, t->link, o+so, 1);
            l = newlist(l, r);
            e++;
        }
        if(t->width == 0)
            t->width = mw+w;
        return l;

    case TUNION:
    case TSTRUCT:
        /*
         * peek ahead to find type of rhs.
         * if its a structure, then treat
         * this element as a variable
         * rather than an aggregate.
         */
        if(isstruct(a, t))
            goto single;

        if(t->width <= 0) {
            diag(Z, "incomplete structure: %s", s->name);
            return Z;
        }
        l = Z;

    again:
        for(t1 = t->link; t1 != T; t1 = t1->down) {
            if(a->op == OARRAY && t1->etype != TARRAY)
                break;
            if(a->op == OELEM) {
                if(t1->sym != a->sym)
                    continue;
                nextinit();
            }
            r = init1(s, t1, o+t1->offset, 1);
            l = newlist(l, r);
            a = peekinit();
            if(a == Z)
                break;
            if(a->op == OELEM)
                goto again;
        }
        if(a && a->op == OELEM)
            diag(a, "structure element not found %F", a);
        return l;

    default:
        diag(Z, "unknown type in initialization: %T to: %s", t, s->name);
        return Z;
    }
}
/*e: function init1 */

/*s: function newlist */
Node*
newlist(Node *l, Node *r)
{
    if(r == Z)
        return l;
    if(l == Z)
        return r;
    return new(OLIST, l, r);
}
/*e: function newlist */

/*s: function sualign */
//@Scheck: not dead, used by cc.y
void sualign(Type *t)
{
    Type *l;
    long o, w;

    o = 0;
    switch(t->etype) {

    case TSTRUCT:
        t->offset = 0;
        w = 0;
        for(l = t->link; l != T; l = l->down) {
            if(l->nbits) {
                if(l->shift <= 0) {
                    l->shift = -l->shift;
                    w = round(w, tfield->width);
                    o = w;
                    w += tfield->width;
                }
                l->offset = o;
            } else {
                if(l->width < 0 ||
                   l->width == 0 && l->down != T)
                    if(l->sym)
                        diag(Z, "incomplete structure element: %s",
                            l->sym->name);
                    else
                        diag(Z, "incomplete structure element");
                w = align(w, l, Ael1);
                l->offset = w;
                w = align(w, l, Ael2);
            }
        }
        w = align(w, t, Asu2);
        t->width = w;
        acidtype(t);
        pickletype(t);
        return;

    case TUNION:
        t->offset = 0;
        w = 0;
        for(l = t->link; l != T; l = l->down) {
            if(l->width <= 0)
                if(l->sym)
                    diag(Z, "incomplete union element: %s",
                        l->sym->name);
                else
                    diag(Z, "incomplete union element");
            l->offset = 0;
            l->shift = 0;
            o = align(align(0, l, Ael1), l, Ael2);
            if(o > w)
                w = o;
        }
        w = align(w, t, Asu2);
        t->width = w;
        acidtype(t);
        pickletype(t);
        return;

    default:
        diag(Z, "unknown type in sualign: %T", t);
        break;
    }
}
/*e: function sualign */

/*s: function round */
long
round(long v, int w)
{
    int r;

    if(w <= 0 || w > 8) {
        diag(Z, "rounding by %d", w);
        w = 1;
    }
    r = v%w;
    if(r)
        v += w-r;
    return v;
}
/*e: function round */

/*s: function ofnproto */
Type*
ofnproto(Node *n)
{
    Type *tl, *tr, *t;

    if(n == Z)
        return T;
    switch(n->op) {
    case OLIST:
        tl = ofnproto(n->left);
        tr = ofnproto(n->right);
        if(tl == T)
            return tr;
        tl->down = tr;
        return tl;

    case ONAME:
        t = copytyp(n->sym->type);
        t->down = T;
        return t;
    }
    return T;
}
/*e: function ofnproto */

/*s: constant ANSIPROTO */
#define	ANSIPROTO	1
/*e: constant ANSIPROTO */
/*s: constant OLDPROTO */
#define	OLDPROTO	2
/*e: constant OLDPROTO */

/*s: function argmark */
//@Scheck: not dead, used by cc.y
void argmark(Node *n, int pass)
{
    Type *t;

    autoffset = align(0, thisfntype->link, Aarg0);
    stkoff = 0;
    for(; n->left != Z; n = n->left) {
        if(n->op != OFUNC || n->left->op != ONAME)
            continue;
        walkparam(n->right, pass);
        /*s: [[argmark()]] if old proto style */
        if(pass != 0 && anyproto(n->right) == OLDPROTO) {
            t = typ(TFUNC, n->left->sym->type->link);
            t->down = typ(TOLD, T);
            t->down->down = ofnproto(n->right);
            tmerge(t, n->left->sym);
            n->left->sym->type = t;
        }
        /*e: [[argmark()]] if old proto style */
        break;
    }
    autoffset = 0;
    stkoff = 0;
}
/*e: function argmark */

/*s: function walkparam */
void
walkparam(Node *n, int pass)
{
    Sym *s;
    Node *n1;

    if(n != Z && n->op == OPROTO && n->left == Z && n->type == types[TVOID])
        return;

loop:
    if(n == Z)
        return;
    switch(n->op) {
    case OLIST:
        walkparam(n->left, pass);
        n = n->right;
        goto loop;

    case OPROTO:
        for(n1 = n; n1 != Z; n1=n1->left)
            if(n1->op == ONAME) {
                if(pass == 0) {
                    s = n1->sym;
                    push1(s);
                    s->offset = -1;
                    break;
                }
                dodecl(pdecl, CPARAM, n->type, n->left);
                break;
            }
        if(n1)
            break;
        if(pass == 0) {
            /*
             * extension:
             *	allow no name in argument declaration
            diag(Z, "no name in argument declaration");
             */
            break;
        }
        dodecl(NODECL, CPARAM, n->type, n->left);
        pdecl(CPARAM, lastdcltype, S);
        break;

    case ODOTDOT:
        break;
    
    case ONAME:
        s = n->sym;
        if(pass == 0) {
            push1(s);
            s->offset = -1;
            break;
        }
        if(s->offset != -1) {
            if(autoffset == 0) {
                firstarg = s;
                firstargtype = s->type;
            }
            autoffset = align(autoffset, s->type, Aarg1);
            s->offset = autoffset;
            autoffset = align(autoffset, s->type, Aarg2);
        } else
            dodecl(pdecl, CXXX, types[TINT], n);
        break;
    default:
        diag(n, "argument not a name/prototype: %O", n->op);
        break;

    }
}
/*e: function walkparam */

/*s: function markdcl */
//@Scheck: used by cc.y
void markdcl(void)
{
    Decl *d;

    blockno++;

    d = push();
    d->val = DMARK;

    d->offset = autoffset;
    d->block = autobn;

    autobn = blockno;
}
/*e: function markdcl */

/*s: function revertdcl */
//@Scheck: used by cc.y
Node* revertdcl(void)
{
    Decl *d;
    Sym *s;
    Node *n;
    /*s: [[revertdcl()]] other locals */
    Node *n1;
    /*e: [[revertdcl()]] other locals */

    n = Z;
    for(;;) {
        d = dclstack;
        if(d == D) {
            diag(Z, "pop off dcl stack");
            break;
        }
        // pop(declstack)
        dclstack = d->link;

        s = d->sym;
        switch(d->val) {
        /*s: [[revertdcl()]] switch declaration type cases */
        case DMARK:
            // restore info previous block
            autoffset = d->offset;
            autobn = d->block;
            // we popped everything, exit loop and return
            return n;
        /*x: [[revertdcl()]] switch declaration type cases */
        case DSUE:
            if(debug['d'])
                print("revert2 \"%s\"\n", s->name);

            // retore info previous tag
            s->suetag = d->type;
            s->sueblock = d->block;

            break;
        /*x: [[revertdcl()]] switch declaration type cases */
        case DLABEL:
            if(debug['d'])
                print("revert3 \"%s\"\n", s->name);

            /*s: [[reverdcl()]] DLABEL case, warn if label not used */
            if(s->label && s->label->addable == 0)
                warn(s->label, "label declared and not used \"%s\"", s->name);
            /*e: [[reverdcl()]] DLABEL case, warn if label not used */
            s->label = Z;
            break;
        /*x: [[revertdcl()]] switch declaration type cases */
        case DAUTO:
            if(debug['d'])
                print("revert1 \"%s\"\n", s->name);

            /*s: [[reverdcl()]] DAUTO case, warn if auto declared but not used */
            if(!s->aused) {
                nearln = s->varlineno;
                if(s->class == CAUTO)
                    warn(Z, "auto declared and not used: %s", s->name);
                if(s->class == CPARAM)
                    warn(Z, "param declared and not used: %s", s->name);
            }
            /*e: [[reverdcl()]] DAUTO case, warn if auto declared but not used */
            /*s: [[reverdcl()]] if volatile symbol */
            if(s->type && (s->type->garb & GVOLATILE)) {
                // add USED(&local_volatile);
                n1 = new(ONAME, Z, Z);
                n1->sym = s;
                n1->type = s->type;
                n1->etype = (s->type != T) ? s->type->etype : TVOID;
                n1->xoffset = s->offset;
                n1->class = s->class;

                n1 = new(OADDR, n1, Z);
                n1 = new(OUSED, n1, Z);
                if(n == Z)
                    n = n1;
                else
                    n = new(OLIST, n1, n);
            }
            /*e: [[reverdcl()]] if volatile symbol */
            // restore info previous identifier
            /*s: [[reverdcl()]] DAUTO case, restore symbol fields from decl */
            s->type = d->type;
            s->offset = d->offset;
            s->block = d->block;

            s->class = d->class;
            s->varlineno = d->varlineno;
            s->aused = d->aused;
            /*e: [[reverdcl()]] DAUTO case, restore symbol fields from decl */

            break;
        /*e: [[revertdcl()]] switch declaration type cases */
        }
    }
    return n;
}
/*e: function revertdcl */

/*s: function fnproto */
Type*
fnproto(Node *n)
{
    /*s: [[fnproto()]] old prototype style handling */
    int r;

    r = anyproto(n->right);
    if(r == 0 || (r & OLDPROTO)) {
        if(r & ANSIPROTO)
            diag(n, "mixed ansi/old function declaration: %F", n->left);
        return T;
    }
    /*e: [[fnproto()]] old prototype style handling */
    return fnproto1(n->right);
}
/*e: function fnproto */

/*s: function anyproto */
bool
anyproto(Node *n)
{
    int r;

    r = false;

loop:
    if(n == Z)
        return r;
    switch(n->op) {
    case OLIST:
        r |= anyproto(n->left);
        n = n->right;
        goto loop;

    case ODOTDOT:
    case OPROTO:
        return r | ANSIPROTO;
    }
    return r | OLDPROTO;
}
/*e: function anyproto */

/*s: function fnproto1 */
Type*
fnproto1(Node *n)
{
    Type *t;

    if(n == Z)
        return T;

    switch(n->op) {
    /*s: [[fnproto1()]] switch node kind cases */
    case ONAME:
        diag(n, "incomplete argument prototype");
        return typ(TINT, T);
    /*x: [[fnproto1()]] switch node kind cases */
    case OPROTO:
        lastdcltype = T;
        dodecl(NODECL, CXXX, n->type, n->left);
        t = typ(TXXX, T);
        if(lastdcltype != T)
            *t = *paramconv(lastdcltype, true);
        return t;
    /*x: [[fnproto1()]] switch node kind cases */
    case OLIST:
        t = fnproto1(n->left);
        if(t != T)
            t->down = fnproto1(n->right);
        return t;
    /*x: [[fnproto1()]] switch node kind cases */
    case ODOTDOT:
        return typ(TDOT, T);
    /*e: [[fnproto1()]] switch node kind cases */
    }
    diag(n, "unknown op in fnproto");
    return T;
}
/*e: function fnproto1 */

/*s: function dbgdecl */
void
dbgdecl(Sym *s)
{
    print("decl \"%s\": C=%s [B=%d:O=%ld] T=%T\n",
        s->name, cnames[s->class], s->block, s->offset, s->type);
}
/*e: function dbgdecl */

/*s: function push */
Decl*
push(void)
{
    Decl *d;

    d = alloc(sizeof(Decl));
    d->link = dclstack;
    dclstack = d;
    return d;
}
/*e: function push */

/*s: function push1 */
Decl*
push1(Sym *s)
{
    Decl *d;

    d = push();
    d->sym = s;
    d->val = DAUTO;

    /*s: [[push1()]] save symbol fields in decl */
    d->type = s->type;
    d->offset = s->offset;
    d->block = s->block;

    d->class = s->class;
    d->varlineno = s->varlineno;
    d->aused = s->aused;
    /*e: [[push1()]] save symbol fields in decl */

    return d;
}
/*e: function push1 */

/*s: function sametype */
bool
sametype(Type *t1, Type *t2)
{

    if(t1 == t2)
        return true;
    return rsametype(t1, t2, 5, true);
}
/*e: function sametype */

/*s: function rsametype */
bool
rsametype(Type *t1, Type *t2, int n, bool f)
{
    int et;

    n--;
    for(;;) {
        if(t1 == t2)
            return true;
        if(t1 == T || t2 == T)
            return false;
        if(n <= 0)
            return true;

        et = t1->etype;
        if(et != t2->etype)
            return false;

        if(et == TFUNC) {
            if(!rsametype(t1->link, t2->link, n, false))
                return false;
            t1 = t1->down;
            t2 = t2->down;
            while(t1 != T && t2 != T) {

                /*s: [[rsametype()]] continue if old style type */
                if(t1->etype == TOLD) {
                    t1 = t1->down;
                    continue;
                }
                if(t2->etype == TOLD) {
                    t2 = t2->down;
                    continue;
                }
                /*e: [[rsametype()]] continue if old style type */

                while(t1 != T || t2 != T) {
                    if(!rsametype(t1, t2, n, false))
                        return false;
                    t1 = t1->down;
                    t2 = t2->down;
                }
                break;
            }
            return true;
        }
        if(et == TARRAY)
            if(t1->width != t2->width && t1->width != 0 && t2->width != 0)
                return false;
        if(typesu[et]) {
            if(t1->link == T)
                snap(t1);
            if(t2->link == T)
                snap(t2);
            if(t1 != t2 && t1->link == T && t2->link == T){
                /* structs with missing or different tag names aren't considered equal */
                if(t1->tag == nil || t2->tag == nil ||
                   strcmp(t1->tag->name, t2->tag->name) != 0)
                    return false;
            }
            t1 = t1->link;
            t2 = t2->link;
            for(;;) {
                if(t1 == t2)
                    return true;
                if(!rsametype(t1, t2, n, false))
                    return false;
                t1 = t1->down;
                t2 = t2->down;
            }
        }
        t1 = t1->link;
        t2 = t2->link;

        if((f || !debug['V']) && et == TIND) {
            if(t1 != T && t1->etype == TVOID)
                return true;
            if(t2 != T && t2->etype == TVOID)
                return true;
        }
    }
}
/*e: function rsametype */

typedef struct Typetab Typetab;

/*s: struct Typetab */
struct Typetab {
    int n;
    Type **a;
};
/*e: struct Typetab */

/*s: function sigind */
static int
sigind(Type *t, Typetab *tt)
{
    int n;
    Type **a, **na, **p, **e;

    n = tt->n;
    a = tt->a;
    e = a+n;
    /* linear search seems ok */
    for(p = a ; p < e; p++)
        if(sametype(*p, t))
            return p-a;
    if((n&15) == 0){
        na = malloc((n+16)*sizeof(Type*));
        memmove(na, a, n*sizeof(Type*));
        free(a);
        a = tt->a = na;
    }
    a[tt->n++] = t;
    return -1;
}
/*e: function sigind */

/*s: function signat */
static ulong
signat(Type *t, Typetab *tt)
{
    int i;
    Type *t1;
    long s;

    s = 0;
    for(; t; t=t->link) {
        s = s*thash1 + thash[t->etype];
        if(t->garb&GINCOMPLETE)
            return s;
        switch(t->etype) {
        default:
            return s;
        case TARRAY:
            s = s*thash2 + 0;	/* was t->width */
            break;
        case TFUNC:
            for(t1=t->down; t1; t1=t1->down)
                s = s*thash3 + signat(t1, tt);
            break;
        case TSTRUCT:
        case TUNION:
            if((i = sigind(t, tt)) >= 0){
                s = s*thash2 + i;
                return s;
            }
            for(t1=t->link; t1; t1=t1->down)
                s = s*thash3 + signat(t1, tt);
            return s;
        case TIND:
            break;
        }
    }
    return s;
}
/*e: function signat */

/*s: function signature */
ulong
signature(Type *t)
{
    ulong s;
    Typetab tt;

    tt.n = 0;
    tt.a = nil;
    s = signat(t, &tt);
    free(tt.a);
    return s;
}
/*e: function signature */

/*s: function sign */
ulong
sign(Sym *s)
{
    ulong v;
    Type *t;

    if(s->sig == SIGINTERN)
        return SIGNINTERN;
    if((t = s->type) == T)
        return 0;
    v = signature(t);
    if(v == 0)
        v = SIGNINTERN;
    return v;
}
/*e: function sign */

/*s: function snap */
void
snap(Type *t)
{
    if(typesu[t->etype])
     if(t->link == T && t->tag && t->tag->suetag) {
        t->link = t->tag->suetag->link;
        t->width = t->tag->suetag->width;
    }
}
/*e: function snap */

/*s: function dotag */
//@Scheck: used by cc.y
Type* dotag(Sym *s, int et, int bn)
{
    /*s: [[dotag()]] if bn not null and bn not sueblock */
    Decl *d;

    if(bn != 0 && bn != s->sueblock) {
        d = push();
        d->sym = s;
        d->val = DSUE;
        d->type = s->suetag;
        d->block = s->sueblock;
        s->suetag = T;
    }
    /*e: [[dotag()]] if bn not null and bn not sueblock */
    if(s->suetag == T) {
        s->suetag = typ(et, T); // link is null for now
        s->sueblock = autobn;
    }
    /*s: [[dotag()]] sanity check tag redeclaration */
    if(s->suetag->etype != et)
        diag(Z, "tag used for more than one type: %s",
            s->name);
    /*e: [[dotag()]] sanity check tag redeclaration */
    if(s->suetag->tag == S)
        s->suetag->tag = s;

    return s->suetag;
}
/*e: function dotag */

/*s: function dcllabel */
//@Scheck: used by cc.y
Node* dcllabel(Sym *s, bool f)
{
    Decl *d, d1;
    Node *n;

    n = s->label;
    if(n != Z) {
        if(f) {
            if(n->complex)
                diag(Z, "label reused: %s", s->name);
            n->complex = true;	// declared
        } else
            n->addable = true;	// used
        return n;
    }

    d = push();
    d->sym = s;
    d->val = DLABEL;

    dclstack = d->link;

    d1 = *firstdcl;
    *firstdcl = *d;
    *d = d1;

    firstdcl->link = d;
    firstdcl = d;

    n = new(OXXX, Z, Z);
    n->sym = s;
    n->complex = f;
    n->addable = !f;

    s->label = n;

    if(debug['d'])
        dbgdecl(s);
    return n;
}
/*e: function dcllabel */

/*s: function paramconv */
Type*
paramconv(Type *t, bool f)
{

    switch(t->etype) {
    case TARRAY:
        t = typ(TIND, t->link);
        t->width = types[TIND]->width;
        break;
    case TFUNC:
        t = typ(TIND, t);
        t->width = types[TIND]->width;
        break;
    /*s: [[paramconv()]] switch etype, adjust type when not f cases */
    case TFLOAT:
        if(!f)
            t = types[TDOUBLE];
        break;

    case TCHAR:
    case TSHORT:
        if(!f)
            t = types[TINT];
        break;

    case TUCHAR:
    case TUSHORT:
        if(!f)
            t = types[TUINT];
        break;
    /*e: [[paramconv()]] switch etype, adjust type when not f cases */
    }
    return t;
}
/*e: function paramconv */

/*s: function adecl */
void
adecl(int c, Type *t, Sym *s)
{

    /*s: [[adecl()]] adjust storage to CLOCAL if static local variable */
    if(c == CSTATIC)
        c = CLOCAL;
    /*e: [[adecl()]] adjust storage to CLOCAL if static local variable */
    if(t->etype == TFUNC) {
        if(c == CXXX)
            c = CEXTERN;
        if(c == CLOCAL)
            c = CSTATIC;
        if(c == CAUTO || c == CEXREG)
            diag(Z, "function cannot be %s %s", cnames[c], s->name);
    }
    if(c == CXXX)
        c = CAUTO;
    if(s) {
        if(s->class == CSTATIC)
            if(c == CEXTERN || c == CGLOBL) {
                warn(Z, "just say static: %s", s->name);
                c = CSTATIC;
            }
        if(s->class == CAUTO || s->class == CPARAM || s->class == CLOCAL)
        if(s->block == autobn)
            diag(Z, "auto redeclaration of: %s", s->name);
        if(c != CPARAM)
            push1(s);
        s->block = autobn;
        s->offset = 0;
        s->type = t;
        s->class = c;
        s->aused = false;
    }

    switch(c) {
    case CAUTO:
        autoffset = align(autoffset, t, Aaut3);
        stkoff = maxround(stkoff, autoffset);
        s->offset = -autoffset;
        break;

    case CPARAM:
        if(autoffset == 0) {
            firstarg = s;
            firstargtype = t;
        }
        autoffset = align(autoffset, t, Aarg1);
        if(s)
            s->offset = autoffset;
        autoffset = align(autoffset, t, Aarg2);
        break;
    }
}
/*e: function adecl */

/*s: function pdecl */
void
pdecl(int class, Type *t, Sym *s)
{
    if(s && s->offset != -1) {
        diag(Z, "not a parameter: %s", s->name);
        return;
    }
    t = paramconv(t, class==CPARAM);
    if(class == CXXX)
        class = CPARAM;
    if(class != CPARAM) {
        diag(Z, "parameter cannot have class: %s", s->name);
        class = CPARAM;
    }
    if(typesu[t->etype] && t->width <= 0)
        diag(Z, "incomplete structure: %s", t->tag->name);
    adecl(class, t, s);
}
/*e: function pdecl */

/*s: function xdecl */
void
xdecl(int class, Type *t, Sym *s)
{
    long o = 0; // offset

    // adjusting class, and possibly o (for CEXREG)
    switch(class) {
    /*s: [[xdecl()]] switch class cases */
    case CXXX:
        class = CGLOBL;
        if(s->class == CEXTERN)
            s->class = CGLOBL;
        break;
    /*x: [[xdecl()]] switch class cases */
    case CEXTERN:
        if(s->class == CGLOBL)
            class = CGLOBL;
        break;
    /*x: [[xdecl()]] switch class cases */
    case CAUTO:
        diag(Z, "overspecified class: %s %s %s", s->name, cnames[class], cnames[s->class]);
        class = CEXTERN;
        break;
    /*x: [[xdecl()]] switch class cases */
    case CEXREG:
        o = exreg(t);
        if(o == 0)
            class = CEXTERN;
        if(s->class == CGLOBL)
            class = CGLOBL;
        break;
    /*x: [[xdecl()]] switch class cases */
    case CTYPESTR:
        if(!typesuv[t->etype]) {
            diag(Z, "typestr must be struct/union: %s", s->name);
            break;
        }
        dclfunct(t, s);
        break;
    /*e: [[xdecl()]] switch class cases */
    }

    /*s: [[xdecl()]] sanity checks */
    if(s->class == CSTATIC)
        if(class == CEXTERN || class == CGLOBL) {
            warn(Z, "overspecified class: %s %s %s", s->name, cnames[class], cnames[s->class]);
            class = CSTATIC;
        }

    if(s->type != T)
        if(s->class != class || !sametype(t, s->type) || t->etype == TENUM) {
            diag(Z, "external redeclaration of: %s", s->name);
            Bprint(&diagbuf, "	%s %T %L\n", cnames[class], t, nearln);
            Bprint(&diagbuf, "	%s %T %L\n", cnames[s->class], s->type, s->varlineno);
        }
    /*e: [[xdecl()]] sanity checks */

    tmerge(t, s);
    s->type = t;
    s->class = class;
    s->block = 0;
    s->offset = o;
}
/*e: function xdecl */

/*s: function tmerge */
void
tmerge(Type *t1, Sym *s)
{
    Type *ta, *tb, *t2;

    t2 = s->type;
/*print("merge	%T; %T\n", t1, t2);/**/
    for(;;) {
        if(t1 == T || t2 == T || t1 == t2)
            break;
        if(t1->etype != t2->etype)
            break;
        switch(t1->etype) {
        case TFUNC:
            ta = t1->down;
            tb = t2->down;
            if(ta == T) {
                t1->down = tb;
                break;
            }
            if(tb == T)
                break;
            while(ta != T && tb != T) {
                if(ta == tb)
                    break;
                /* ignore old-style flag */
                if(ta->etype == TOLD) {
                    ta = ta->down;
                    continue;
                }
                if(tb->etype == TOLD) {
                    tb = tb->down;
                    continue;
                }
                /* checking terminated by ... */
                if(ta->etype == TDOT && tb->etype == TDOT) {
                    ta = T;
                    tb = T;
                    break;
                }
                if(!sametype(ta, tb))
                    break;
                ta = ta->down;
                tb = tb->down;
            }
            if(ta != tb)
                diag(Z, "function inconsistently declared: %s", s->name);

            /* take new-style over old-style */
            ta = t1->down;
            tb = t2->down;
            if(ta != T && ta->etype == TOLD)
                if(tb != T && tb->etype != TOLD)
                    t1->down = tb;
            break;

        case TARRAY:
            /* should we check array size change? */
            if(t2->width > t1->width)
                t1->width = t2->width;
            break;

        case TUNION:
        case TSTRUCT:
            return;
        }
        t1 = t1->link;
        t2 = t2->link;
    }
}
/*e: function tmerge */

/*s: function edecl */
//@Scheck: used by cc.y
void edecl(int c, Type *t, Sym *s)
{
    Type *t1;

    if(s == S) {
        if(!typesu[t->etype])
            diag(Z, "unnamed structure element must be struct/union");
        if(c != CXXX)
            diag(Z, "unnamed structure element cannot have class");
    } else
        if(c != CXXX)
            diag(Z, "structure element cannot have class: %s", s->name);
    t1 = t;
    t = copytyp(t1);
    t->sym = s;
    t->down = T;
    if(lastfield) {
        t->shift = lastbit - lastfield;
        t->nbits = lastfield;
        if(firstbit)
            t->shift = -t->shift;
        if(typeu[t->etype])
            t->etype = tufield->etype;
        else
            t->etype = tfield->etype;
    }
    if(strf == T)
        strf = t;
    else
        strl->down = t;
    strl = t;
}
/*e: function edecl */

/*s: function maxtype */
/*
 * this routine is very suspect.
 * ANSI requires the enum type to
 * be represented as an 'int'
 * this means that 0x81234567
 * would be illegal. this routine
 * makes signed and unsigned go
 * to unsigned.
 */
Type*
maxtype(Type *t1, Type *t2)
{

    if(t1 == T)
        return t2;
    if(t2 == T)
        return t1;
    if(t1->etype > t2->etype)
        return t1;
    return t2;
}
/*e: function maxtype */

/*s: function doenum */
//@Scheck: used by cc.y
void doenum(Sym *s, Node *n)
{

    if(n) {
        complex(n); // will call evconst()
        if(n->op != OCONST) {
            diag(n, "enum not a constant: %s", s->name);
            return;
        }

        en.cenum = n->type;
        en.tenum = maxtype(en.cenum, en.tenum);

        if(typefd[en.cenum->etype])
            en.floatenum = n->fconst;
        else
            en.lastenum = n->vconst;
    }
    if(dclstack)
        push1(s);

    xdecl(CXXX, types[TENUM], s);

    if(en.cenum == T) {
        en.tenum = types[TINT];
        en.cenum = types[TINT];
        en.lastenum = 0;
    }
    s->tenum = en.cenum;

    if(typefd[s->tenum->etype]) {
        s->fconst = en.floatenum;
        en.floatenum++;
    } else {
        s->vconst = convvtox(en.lastenum, s->tenum->etype);
        en.lastenum++;
    }

    if(debug['d'])
        dbgdecl(s);
    acidvar(s);
}
/*e: function doenum */

/*s: function symadjust */
void
symadjust(Sym *s, Node *n, long del)
{

    switch(n->op) {
    case ONAME:
        if(n->sym == s)
            n->xoffset -= del;
        return;

    case OCONST:
    case OSTRING: case OLSTRING:
    case OREGISTER: case OINDREG:
        return;

    default:
        if(n->left)
            symadjust(s, n->left, del);
        if(n->right)
            symadjust(s, n->right, del);
        return;

    }
}
/*e: function symadjust */

/*s: function contig */
//@Scheck: used by cc.y
Node* contig(Sym *s, Node *n, long v)
{
    Node *p, *r, *q, *m;
    long w;
    Type *zt;

    if(debug['i']) {
        print("contig v = %ld; s = %s\n", v, s->name);
        prtree(n, "doinit value");
    }

    if(n == Z)
        goto no;
    w = s->type->width;

    /*
     * nightmare: an automatic array whose size
     * increases when it is initialized
     */
    if(v != w) {
        if(v != 0)
            diag(n, "automatic adjustable array: %s", s->name);
        v = s->offset;
        autoffset = align(autoffset, s->type, Aaut3);
        s->offset = -autoffset;
        stkoff = maxround(stkoff, autoffset);
        symadjust(s, n, v - s->offset);
    }
    if(w <= ewidth[TIND])
        goto no;
    if(n->op == OAS)
        diag(Z, "oops in contig");
/*ZZZ this appears incorrect
need to check if the list completely covers the data.
if not, bail
 */
    if(n->op == OLIST)
        goto no;
    if(n->op == OASI)
        if(n->left->type)
        if(n->left->type->width == w)
            goto no;
    while(w & (ewidth[TIND]-1))
        w++;
/*
 * insert the following code, where long becomes vlong if pointers are fat
 *
    *(long**)&X = (long*)((char*)X + sizeof(X));
    do {
        *(long**)&X -= 1;
        **(long**)&X = 0;
    } while(*(long**)&X);
 */

    for(q=n; q->op != ONAME; q=q->left)
        ;

    zt = ewidth[TIND] > ewidth[TLONG]? types[TVLONG]: types[TLONG];

    p = new(ONAME, Z, Z);
    *p = *q;
    p->type = typ(TIND, zt);
    p->xoffset = s->offset;

    r = new(ONAME, Z, Z);
    *r = *p;
    r = new(OPOSTDEC, r, Z);

    q = new(ONAME, Z, Z);
    *q = *p;
    q = new(OIND, q, Z);

    m = new(OCONST, Z, Z);
    m->vconst = 0;
    m->type = zt;

    q = new(OAS, q, m);

    r = new(OLIST, r, q);

    q = new(ONAME, Z, Z);
    *q = *p;
    r = new(ODWHILE, q, r);

    q = new(ONAME, Z, Z);
    *q = *p;
    q->type = q->type->link;
    q->xoffset += w;
    q = new(OADDR, q, 0);

    q = new(OASI, p, q);
    r = new(OLIST, q, r);

    n = new(OLIST, r, n);

no:
    return n;
}
/*e: function contig */
/*e: cc/dcl.c */
