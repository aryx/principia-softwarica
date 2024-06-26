/*s: 8c/div.c */
#include "gc.h"

/*s: function [[TN]](x86) */
/*
 * Based on: Granlund, T.; Montgomery, P.L.
 * "Division by Invariant Integers using Multiplication".
 * SIGPLAN Notices, Vol. 29, June 1994, page 61.
 */

#define	TN(n)	((uvlong)1 << (n))
/*e: function [[TN]](x86) */
/*s: constant [[T31]](x86) */
#define	T31	TN(31)
/*e: constant [[T31]](x86) */
/*s: constant [[T32]](x86) */
#define	T32	TN(32)
/*e: constant [[T32]](x86) */

/*s: function [[multiplier]](x86) */
int
multiplier(ulong d, int p, uvlong *mp)
{
    int l;
    uvlong mlo, mhi, tlo, thi;

    l = topbit(d - 1) + 1;
    mlo = (((TN(l) - d) << 32) / d) + T32;
    if(l + p == 64)
        mhi = (((TN(l) + 1 - d) << 32) / d) + T32;
    else
        mhi = (TN(32 + l) + TN(32 + l - p)) / d;
    assert(mlo < mhi);
    while(l > 0) {
        tlo = mlo >> 1;
        thi = mhi >> 1;
        if(tlo == thi)
            break;
        mlo = tlo;
        mhi = thi;
        l--;
    }
    *mp = mhi;
    return l;
}
/*e: function [[multiplier]](x86) */

/*s: function [[sdiv]](x86) */
int
sdiv(ulong d, ulong *mp, int *sp)
{
    int s;
    uvlong m;

    s = multiplier(d, 32 - 1, &m);
    *mp = m;
    *sp = s;
    if(m >= T31)
        return 1;
    else
        return 0;
}
/*e: function [[sdiv]](x86) */

/*s: function [[udiv]](x86) */
int
udiv(ulong d, ulong *mp, int *sp, int *pp)
{
    int p, s;
    uvlong m;

    s = multiplier(d, 32, &m);
    p = 0;
    if(m >= T32) {
        while((d & 1) == 0) {
            d >>= 1;
            p++;
        }
        s = multiplier(d, 32 - p, &m);
    }
    *mp = m;
    *pp = p;
    if(m >= T32) {
        assert(p == 0);
        *sp = s - 1;
        return 1;
    }
    else {
        *sp = s;
        return 0;
    }
}
/*e: function [[udiv]](x86) */

/*s: function [[sdivgen]](x86) */
void
sdivgen(Node *l, Node *r, Node *ax, Node *dx)
{
    int a, s;
    ulong m;
    vlong c;

    c = r->vconst;
    if(c < 0)
        c = -c;
    a = sdiv(c, &m, &s);
//print("a=%d i=%ld s=%d m=%lux\n", a, (long)r->vconst, s, m);
    gins(AMOVL, nodconst(m), ax);
    gins(AIMULL, l, Z);
    gins(AMOVL, l, ax);
    if(a)
        gins(AADDL, ax, dx);
    gins(ASHRL, nodconst(31), ax);
    gins(ASARL, nodconst(s), dx);
    gins(AADDL, ax, dx);
    if(r->vconst < 0)
        gins(ANEGL, Z, dx);
}
/*e: function [[sdivgen]](x86) */

/*s: function [[udivgen]](x86) */
void
udivgen(Node *l, Node *r, Node *ax, Node *dx)
{
    int a, s, t;
    ulong m;
    Node nod;

    a = udiv(r->vconst, &m, &s, &t);
//print("a=%ud i=%ld p=%d s=%d m=%lux\n", a, (long)r->vconst, t, s, m);
    if(t != 0) {
        gins(AMOVL, l, ax);
        gins(ASHRL, nodconst(t), ax);
        gins(AMOVL, nodconst(m), dx);
        gins(AMULL, dx, Z);
    }
    else if(a) {
        if(l->op != OREGISTER) {
            regalloc(&nod, l, Z);
            gins(AMOVL, l, &nod);
            l = &nod;
        }
        gins(AMOVL, nodconst(m), ax);
        gins(AMULL, l, Z);
        gins(AADDL, l, dx);
        gins(ARCRL, nodconst(1), dx);
        if(l == &nod)
            regfree(l);
    }
    else {
        gins(AMOVL, nodconst(m), ax);
        gins(AMULL, l, Z);
    }
    if(s != 0)
        gins(ASHRL, nodconst(s), dx);
}
/*e: function [[udivgen]](x86) */

/*s: function [[sext]](x86) */
void
sext(Node *d, Node *s, Node *l)
{
    if(s->reg == D_AX && !nodreg(d, Z, D_DX)) {
        reg[D_DX]++;
        gins(ACDQ, Z, Z);
    }
    else {
        regalloc(d, l, Z);
        gins(AMOVL, s, d);
        gins(ASARL, nodconst(31), d);
    }
}
/*e: function [[sext]](x86) */

/*s: function [[sdiv2]](x86) */
void
sdiv2(long c, int v, Node *l, Node *n)
{
    Node nod;

    if(v > 0) {
        if(v > 1) {
            sext(&nod, n, l);
            gins(AANDL, nodconst((1 << v) - 1), &nod);
            gins(AADDL, &nod, n);
            regfree(&nod);
        }
        else {
            gins(ACMPL, n, nodconst(0x80000000));
            gins(ASBBL, nodconst(-1), n);
        }
        gins(ASARL, nodconst(v), n);
    }
    if(c < 0)
        gins(ANEGL, Z, n);
}
/*e: function [[sdiv2]](x86) */

/*s: function [[smod2]](x86) */
void
smod2(long c, int v, Node *l, Node *n)
{
    Node nod;

    if(c == 1) {
        zeroregm(n);
        return;
    }

    sext(&nod, n, l);
    if(v == 0) {
        zeroregm(n);
        gins(AXORL, &nod, n);
        gins(ASUBL, &nod, n);
    }
    else if(v > 1) {
        gins(AANDL, nodconst((1 << v) - 1), &nod);
        gins(AADDL, &nod, n);
        gins(AANDL, nodconst((1 << v) - 1), n);
        gins(ASUBL, &nod, n);
    }
    else {
        gins(AANDL, nodconst(1), n);
        gins(AXORL, &nod, n);
        gins(ASUBL, &nod, n);
    }
    regfree(&nod);
}
/*e: function [[smod2]](x86) */
/*e: 8c/div.c */
