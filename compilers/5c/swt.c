/*s: 5c/swt.c */
#include "gc.h"

/*s: function swit1(arm) */
void
swit1(C1 *q, int nc, long def, Node *n)
{
    Node tn;
    
    regalloc(&tn, &regnode, Z);
    swit2(q, nc, def, n, &tn);
    regfree(&tn);
}
/*e: function swit1(arm) */

/*s: function swit2(arm) */
void
swit2(C1 *q, int nc, long def, Node *n, Node *tn)
{
    C1 *r;
    int i;
    long v;
    Prog *sp;

    if(nc >= 3) {
        i = (q+nc-1)->val - (q+0)->val;
        if(i > 0 && i < nc*2)
            goto direct;
    }
    if(nc < 5) {
        for(i=0; i<nc; i++) {
            if(debug['K'])
                print("case = %.8llux\n", q->val);
            gopcode(OEQ, nodconst(q->val), n, Z);
            patch(p, q->label);
            q++;
        }
        gbranch(OGOTO);
        patch(p, def);

        return;
    }

    i = nc / 2;
    r = q+i;
    if(debug['K'])
        print("case > %.8llux\n", r->val);
    gopcode(OGT, nodconst(r->val), n, Z);
    sp = p;
    gopcode(OEQ, nodconst(r->val), n, Z);	/* just gen the B.EQ */
    patch(p, r->label);
    swit2(q, i, def, n, tn);

    if(debug['K'])
        print("case < %.8llux\n", r->val);
    patch(sp, pc);
    swit2(r+1, nc-i-1, def, n, tn);
    return;

direct:
    v = q->val;
    if(v != 0)
        gopcode(OSUB, nodconst(v), Z, n);
    gopcode(OCASE, nodconst((q+nc-1)->val - v), n, Z);
    patch(p, def);
    for(i=0; i<nc; i++) {
        if(debug['K'])
            print("case = %.8llux\n", q->val);
        while(q->val != v) {
            nextpc();
            p->as = ABCASE;
            patch(p, def);
            v++;
        }
        nextpc();
        p->as = ABCASE;
        patch(p, q->label);
        q++;
        v++;
    }

    gbranch(OGOTO);		/* so that regopt() won't be confused */
    patch(p, def);
}
/*e: function swit2(arm) */

/*s: function bitload(arm) */
void
bitload(Node *b, Node *n1, Node *n2, Node *n3, Node *nn)
{
    int sh;
    long v;
    Node *l;

    /*
     * n1 gets adjusted/masked value
     * n2 gets address of cell
     * n3 gets contents of cell
     */
    l = b->left;
    if(n2 != Z) {
        regalloc(n1, l, nn);
        reglcgen(n2, l, Z);
        regalloc(n3, l, Z);
        gopcode(OAS, n2, Z, n3);
        gopcode(OAS, n3, Z, n1);
    } else {
        regalloc(n1, l, nn);
        cgen(l, n1);
    }
    if(b->type->shift == 0 && typeu[b->type->etype]) {
        v = ~0 + (1L << b->type->nbits);
        gopcode(OAND, nodconst(v), Z, n1);
    } else {
        sh = 32 - b->type->shift - b->type->nbits;
        if(sh > 0)
            gopcode(OASHL, nodconst(sh), Z, n1);
        sh += b->type->shift;
        if(sh > 0)
            if(typeu[b->type->etype])
                gopcode(OLSHR, nodconst(sh), Z, n1);
            else
                gopcode(OASHR, nodconst(sh), Z, n1);
    }
}
/*e: function bitload(arm) */

/*s: function bitstore(arm) */
void
bitstore(Node *b, Node *n1, Node *n2, Node *n3, Node *nn)
{
    long v;
    Node nod, *l;
    int sh;

    /*
     * n1 has adjusted/masked value
     * n2 has address of cell
     * n3 has contents of cell
     */
    l = b->left;
    regalloc(&nod, l, Z);
    v = ~0 + (1L << b->type->nbits);
    gopcode(OAND, nodconst(v), Z, n1);
    gopcode(OAS, n1, Z, &nod);
    if(nn != Z)
        gopcode(OAS, n1, Z, nn);
    sh = b->type->shift;
    if(sh > 0)
        gopcode(OASHL, nodconst(sh), Z, &nod);
    v <<= sh;
    gopcode(OAND, nodconst(~v), Z, n3);
    gopcode(OOR, n3, Z, &nod);
    gopcode(OAS, &nod, Z, n2);

    regfree(&nod);
    regfree(n1);
    regfree(n2);
    regfree(n3);
}
/*e: function bitstore(arm) */

/*s: function outstring(arm) */
long
outstring(char *s, long n)
{
    long r;

    if(suppress)
        return nstring;
    r = nstring;
    while(n) {
        string[mnstring] = *s++;
        mnstring++;
        nstring++;
        if(mnstring >= NSNAME) {
            gpseudo(ADATA, symstring, nodconst(0L));
            p->from.offset += nstring - NSNAME;
            p->reg = NSNAME;
            p->to.type = D_SCONST;
            memmove(p->to.sval, string, NSNAME);
            mnstring = 0;
        }
        n--;
    }
    return r;
}
/*e: function outstring(arm) */

/*s: function mulcon(arm) */
int
mulcon(Node *n, Node *nn)
{
    Node *l, *r, nod1, nod2;
    Multab *m;
    long v, vs;
    int o;
    char code[sizeof(m->code)+2], *p;

    if(typefd[n->type->etype])
        return 0;
    l = n->left;
    r = n->right;
    if(l->op == OCONST) {
        l = r;
        r = n->left;
    }
    if(r->op != OCONST)
        return 0;
    v = convvtox(r->vconst, n->type->etype);
    if(v != r->vconst) {
        if(debug['M'])
            print("%L multiply conv: %lld\n", n->lineno, r->vconst);
        return 0;
    }
    m = mulcon0(v);
    if(!m) {
        if(debug['M'])
            print("%L multiply table: %lld\n", n->lineno, r->vconst);
        return 0;
    }
    if(debug['M'] && debug['v'])
        print("%L multiply: %ld\n", n->lineno, v);

    memmove(code, m->code, sizeof(m->code));
    code[sizeof(m->code)] = 0;

    p = code;
    if(p[1] == 'i')
        p += 2;
    regalloc(&nod1, n, nn);
    cgen(l, &nod1);
    vs = v;
    regalloc(&nod2, n, Z);

loop:
    switch(*p) {
    case 0:
        regfree(&nod2);
        if(vs < 0) {
            gopcode(OAS, &nod1, Z, &nod1);
            gopcode(OSUB, &nod1, nodconst(0), nn);
        } else 
            gopcode(OAS, &nod1, Z, nn);
        regfree(&nod1);
        return 1;
    case '+':
        o = OADD;
        goto addsub;
    case '-':
        o = OSUB;
    addsub:	/* number is r,n,l */
        v = p[1] - '0';
        r = &nod1;
        if(v&4)
            r = &nod2;
        n = &nod1;
        if(v&2)
            n = &nod2;
        l = &nod1;
        if(v&1)
            l = &nod2;
        gopcode(o, l, n, r);
        break;
    default: /* op is shiftcount, number is r,l */
        v = p[1] - '0';
        r = &nod1;
        if(v&2)
            r = &nod2;
        l = &nod1;
        if(v&1)
            l = &nod2;
        v = *p - 'a';
        if(v < 0 || v >= 32) {
            diag(n, "mulcon unknown op: %c%c", p[0], p[1]);
            break;
        }
        gopcode(OASHL, nodconst(v), l, r);
        break;
    }
    p += 2;
    goto loop;
}
/*e: function mulcon(arm) */

/*s: function gextern(arm) */
void
gextern(Sym *s, Node *a, long o, long w)
{
    /*s: [[gextern()]] if OCONST and a vlong type */
    if(a->op == OCONST && typev[a->type->etype]) {
        if(align(0, types[TCHAR], Aarg1))	/* isbigendian */
            gpseudo(ADATA, s, nod32const(a->vconst>>32));
        else
            gpseudo(ADATA, s, nod32const(a->vconst));
        p->from.offset += o;
        p->reg = 4;
        if(align(0, types[TCHAR], Aarg1))	/* isbigendian */
            gpseudo(ADATA, s, nod32const(a->vconst));
        else
            gpseudo(ADATA, s, nod32const(a->vconst>>32));
        p->from.offset += o + 4;
        p->reg = 4;
    }
    /*e: [[gextern()]] if OCONST and a vlong type */
    else {
        gpseudo(ADATA, s, a);
        p->from.offset += o;
        p->reg = w; // attributes
        if(p->to.type == D_OREG)
            p->to.type = D_CONST; // D_ADDR?
    }
}
/*e: function gextern(arm) */

void	zname(Biobuf*, Sym*, int);
char*	zaddr(char*, Adr*, int);
void	zwrite(Biobuf*, Prog*, int, int);
void	outhist(Biobuf*);

/*s: function zwrite(arm) */
void
zwrite(Biobuf *b, Prog *p, int sf, int st)
{
    char bf[100], *bp;

    bf[0] = p->as;
    bf[1] = p->scond;
    bf[2] = p->reg;
    bf[3] = p->lineno;
    bf[4] = p->lineno>>8;
    bf[5] = p->lineno>>16;
    bf[6] = p->lineno>>24;
    bp = zaddr(bf+7, &p->from, sf);
    bp = zaddr(bp, &p->to, st);
    Bwrite(b, bf, bp-bf);
}
/*e: function zwrite(arm) */

/*s: struct Htab */
struct Htab { 
    Sym *sym; 
    short type; 
};
/*e: struct Htab */

/*s: function outcode(arm) */
void
outcode(void)
{
    struct Htab h[NSYM];
    Prog *p;
    Sym *s;
    int sf, st, t, sym;

    if(debug['S']) {
        for(p = firstp; p != P; p = p->link)
            if(p->as != ADATA && p->as != AGLOBL)
                pc--;
        for(p = firstp; p != P; p = p->link) {
            print("%P\n", p);
            if(p->as != ADATA && p->as != AGLOBL)
                pc++;
        }
    }
    outhist(&outbuf);
    for(sym=0; sym<NSYM; sym++) {
        h[sym].sym = S;
        h[sym].type = 0;
    }
    sym = 1;
    for(p = firstp; p != P; p = p->link) {
    jackpot:
        sf = 0;
        s = p->from.sym;
        while(s != S) {
            sf = s->symidx;
            if(sf < 0 || sf >= NSYM)
                sf = 0;
            t = p->from.symkind;
            if(h[sf].type == t)
            if(h[sf].sym == s)
                break;
            s->symidx = sym;
            zname(&outbuf, s, t);
            h[sym].sym = s;
            h[sym].type = t;
            sf = sym;
            sym++;
            if(sym >= NSYM)
                sym = 1;
            break;
        }
        st = 0;
        s = p->to.sym;
        while(s != S) {
            st = s->symidx;
            if(st < 0 || st >= NSYM)
                st = 0;
            t = p->to.symkind;
            if(h[st].type == t)
            if(h[st].sym == s)
                break;
            s->symidx = sym;
            zname(&outbuf, s, t);
            h[sym].sym = s;
            h[sym].type = t;
            st = sym;
            sym++;
            if(sym >= NSYM)
                sym = 1;
            if(st == sf)
                goto jackpot;
            break;
        }
        zwrite(&outbuf, p, sf, st);
    }
    firstp = P;
    lastp = P;
}
/*e: function outcode(arm) */

/*s: function outhist(arm) */
void
outhist(Biobuf *b)
{
    Hist *h;
    char *p, *q, *op, c;
    Prog pg;
    int n;

    pg = zprog;
    pg.as = AHISTORY;
    c = pathchar();
    for(h = hist; h != H; h = h->link) {
        p = h->name;
        op = 0;
        /* on windows skip drive specifier in pathname */
        //if(systemtype(Windows) && p && p[1] == ':'){
        //	p += 2;
        //	c = *p;
        //}
        if(p && p[0] != c && h->offset == 0 && pathname){
            /* on windows skip drive specifier in pathname */
            //if(systemtype(Windows) && pathname[1] == ':') {
            //	op = p;
            //	p = pathname+2;
            //	c = *p;
            //} else 
            if(pathname[0] == c){
                op = p;
                p = pathname;
            }
        }
        while(p) {
            q = utfrune(p, c);
            if(q) {
                n = q-p;
                if(n == 0){
                    n = 1;	/* leading "/" */
                    *p = '/';	/* don't emit "\" on windows */
                }
                q++;
            } else {
                n = strlen(p);
                q = 0;
            }
            if(n) {
                Bputc(b, ANAME);
                Bputc(b, D_FILE);
                Bputc(b, 1);
                Bputc(b, '<');
                Bwrite(b, p, n);
                Bputc(b, 0);
            }
            p = q;
            if(p == 0 && op) {
                p = op;
                op = 0;
            }
        }
        pg.lineno = h->line;
        pg.to.type = zprog.to.type;
        pg.to.offset = h->offset;
        if(h->offset)
            pg.to.type = D_CONST;

        zwrite(b, &pg, 0, 0);
    }
}
/*e: function outhist(arm) */

/*s: function zname(arm) */
void
zname(Biobuf *b, Sym *s, int t)
{
    char *n, bf[7];
    ulong sig;

    n = s->name;
    /*s: [[zname()]] if generate signature for symbol s */
    if(debug['T'] && 
       t == N_EXTERN && 
       s->sig != SIGDONE && 
       s->type != types[TENUM] && 
       s != symrathole){

        sig = sign(s);
        bf[0] = ASIGNAME;
        bf[1] = sig;
        bf[2] = sig>>8;
        bf[3] = sig>>16;
        bf[4] = sig>>24;
        bf[5] = t;
        bf[6] = s->symidx;
        Bwrite(b, bf, 7);
        s->sig = SIGDONE;
    }
    /*e: [[zname()]] if generate signature for symbol s */
    else{
        bf[0] = ANAME;
        bf[1] = t;	/* type */
        bf[2] = s->symidx;	/* sym */
        Bwrite(b, bf, 3);
    }
    Bwrite(b, n, strlen(n)+1);
}
/*e: function zname(arm) */

/*s: function zaddr(arm) */
char*
zaddr(char *bp, Adr *a, int s)
{
    long l;
    Ieee e;

    bp[0] = a->type;
    bp[1] = a->reg;
    bp[2] = s;
    bp[3] = a->symkind;
    bp += 4;

    if(a->type == D_CONST && a->symkind != N_NONE) {
        diag(Z, "missing D_CONST -> D_ADDR migration");
    }

    switch(a->type) {
    case D_NONE:
    case D_REG:
    case D_FREG:
    case D_PSR:
        break;

    case D_OREG:
    case D_CONST:
    case D_ADDR:
    case D_BRANCH:
    case D_SHIFT:
        l = a->offset;
        bp[0] = l;
        bp[1] = l>>8;
        bp[2] = l>>16;
        bp[3] = l>>24;
        bp += 4;
        break;

    case D_SCONST:
        memmove(bp, a->sval, NSNAME);
        bp += NSNAME;
        break;

    case D_FCONST:
        ieeedtod(&e, a->dval);
        l = e.l;
        bp[0] = l;
        bp[1] = l>>8;
        bp[2] = l>>16;
        bp[3] = l>>24;
        bp += 4;
        l = e.h;
        bp[0] = l;
        bp[1] = l>>8;
        bp[2] = l>>16;
        bp[3] = l>>24;
        bp += 4;
        break;
    default:
        diag(Z, "unknown type %d in zaddr", a->type);

    }
    return bp;
}
/*e: function zaddr(arm) */

/*s: function align(arm) */
long
align(long i, Type *t, int op)
{
    long o;
    Type *v;
    int w;

    o = i;
    w = 1;
    switch(op) {
    default:
        diag(Z, "unknown align opcode %d", op);
        break;

    case Asu2:	/* padding at end of a struct */
        w = SZ_LONG;
        if(packflg)
            w = packflg;
        break;

    case Ael1:	/* initial align of struct element */
        for(v=t; v->etype==TARRAY; v=v->link)
            ;
        w = ewidth[v->etype];
        if(w <= 0 || w >= SZ_LONG)
            w = SZ_LONG;
        if(packflg)
            w = packflg;
        break;

    case Ael2:	/* width of a struct element */
        o += t->width;
        break;

    case Aarg0:	/* initial passbyptr argument in arg list */
        if(typesuv[t->etype]) {
            o = align(o, types[TIND], Aarg1);
            o = align(o, types[TIND], Aarg2);
        }
        break;

    case Aarg1:	/* initial align of parameter */
        w = ewidth[t->etype];
        if(w <= 0 || w >= SZ_LONG) {
            w = SZ_LONG;
            break;
        }
        w = 1;		/* little endian no adjustment */
        break;

    case Aarg2:	/* width of a parameter */
        o += t->width;
        w = SZ_LONG;
        break;

    case Aaut3:	/* total align of automatic */
        o = align(o, t, Ael2);
        o = align(o, t, Ael1);
        w = SZ_LONG;	/* because of a pun in cc/dcl.c:contig() */
        break;
    }
    o = round(o, w);
    if(debug['A'])
        print("align %s %ld %T = %ld\n", bnames[op], i, t, o);
    return o;
}
/*e: function align(arm) */

/*s: function maxround */
long
maxround(long max, long v)
{
    v = round(v, SZ_LONG);
    if(v > max)
        return v;
    return max;
}
/*e: function maxround */
/*e: 5c/swt.c */
