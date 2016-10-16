/*s: arm/vlrt.c */
typedef unsigned long   ulong;
typedef unsigned int    uint;
typedef unsigned short  ushort;
typedef unsigned char   uchar;
typedef signed char schar;

/*s: macro SIGN (arm) */
#define SIGN(n) (1UL<<(n-1))
/*e: macro SIGN (arm) */

typedef struct  Vlong   Vlong;
/*s: struct Vlong (arm) */
struct  Vlong
{
    ulong   lo;
    ulong   hi;
};
/*e: struct Vlong (arm) */

void    abort(void);

/* needed by profiler; can't be profiled */
#pragma profile off

/*s: function _addv (arm) */
void
_addv(Vlong *r, Vlong a, Vlong b)
{
    ulong lo, hi;

    lo = a.lo + b.lo;
    hi = a.hi + b.hi;
    if(lo < a.lo)
        hi++;
    r->lo = lo;
    r->hi = hi;
}
/*e: function _addv (arm) */

/*s: function _subv (arm) */
void
_subv(Vlong *r, Vlong a, Vlong b)
{
    ulong lo, hi;

    lo = a.lo - b.lo;
    hi = a.hi - b.hi;
    if(lo > a.lo)
        hi--;
    r->lo = lo;
    r->hi = hi;
}
/*e: function _subv (arm) */

#pragma profile on

/*s: function _d2v (arm) */
void
_d2v(Vlong *y, double d)
{
    union { double d; struct Vlong; } x;
    ulong xhi, xlo, ylo, yhi;
    int sh;

    x.d = d;

    xhi = (x.hi & 0xfffff) | 0x100000;
    xlo = x.lo;
    sh = 1075 - ((x.hi >> 20) & 0x7ff);

    ylo = 0;
    yhi = 0;
    if(sh >= 0) {
        /* v = (hi||lo) >> sh */
        if(sh < 32) {
            if(sh == 0) {
                ylo = xlo;
                yhi = xhi;
            } else {
                ylo = (xlo >> sh) | (xhi << (32-sh));
                yhi = xhi >> sh;
            }
        } else {
            if(sh == 32) {
                ylo = xhi;
            } else
            if(sh < 64) {
                ylo = xhi >> (sh-32);
            }
        }
    } else {
        /* v = (hi||lo) << -sh */
        sh = -sh;
        if(sh <= 10) {
            ylo = xlo << sh;
            yhi = (xhi << sh) | (xlo >> (32-sh));
        } else {
            /* overflow */
            yhi = d;    /* causes something awful */
        }
    }
    if(x.hi & SIGN(32)) {
        if(ylo != 0) {
            ylo = -ylo;
            yhi = ~yhi;
        } else
            yhi = -yhi;
    }

    y->hi = yhi;
    y->lo = ylo;
}
/*e: function _d2v (arm) */

/*s: function _f2v (arm) */
void
_f2v(Vlong *y, float f)
{
    _d2v(y, f);
}
/*e: function _f2v (arm) */

/*s: function _v2d (arm) */
double
_v2d(Vlong x)
{
    if(x.hi & SIGN(32)) {
        if(x.lo) {
            x.lo = -x.lo;
            x.hi = ~x.hi;
        } else
            x.hi = -x.hi;
        return -((long)x.hi*4294967296. + x.lo);
    }
    return (long)x.hi*4294967296. + x.lo;
}
/*e: function _v2d (arm) */

/*s: function _v2f (arm) */
float
_v2f(Vlong x)
{
    return _v2d(x);
}
/*e: function _v2f (arm) */

/* too many of these are also needed by profiler; leave them out */
#pragma profile off

/*s: function dodiv (arm) */
static void
dodiv(Vlong num, Vlong den, Vlong *q, Vlong *r)
{
    ulong numlo, numhi, denhi, denlo, quohi, quolo, t;
    int i;

    numhi = num.hi;
    numlo = num.lo;
    denhi = den.hi;
    denlo = den.lo;
    /*
     * get a divide by zero
     */
    if(denlo==0 && denhi==0) {
        numlo = numlo / denlo;
    }

    /*
     * set up the divisor and find the number of iterations needed
     */
    if(numhi >= SIGN(32)) {
        quohi = SIGN(32);
        quolo = 0;
    } else {
        quohi = numhi;
        quolo = numlo;
    }
    i = 0;
    while(denhi < quohi || (denhi == quohi && denlo < quolo)) {
        denhi = (denhi<<1) | (denlo>>31);
        denlo <<= 1;
        i++;
    }

    quohi = 0;
    quolo = 0;
    for(; i >= 0; i--) {
        quohi = (quohi<<1) | (quolo>>31);
        quolo <<= 1;
        if(numhi > denhi || (numhi == denhi && numlo >= denlo)) {
            t = numlo;
            numlo -= denlo;
            if(numlo > t)
                numhi--;
            numhi -= denhi;
            quolo |= 1;
        }
        denlo = (denlo>>1) | (denhi<<31);
        denhi >>= 1;
    }

    if(q) {
        q->lo = quolo;
        q->hi = quohi;
    }
    if(r) {
        r->lo = numlo;
        r->hi = numhi;
    }
}
/*e: function dodiv (arm) */

/*s: function _divvu (arm) */
void
_divvu(Vlong *q, Vlong n, Vlong d)
{
    if(n.hi == 0 && d.hi == 0) {
        q->hi = 0;
        q->lo = n.lo / d.lo;
        return;
    }
    dodiv(n, d, q, 0);
}
/*e: function _divvu (arm) */

/*s: function _modvu (arm) */
void
_modvu(Vlong *r, Vlong n, Vlong d)
{

    if(n.hi == 0 && d.hi == 0) {
        r->hi = 0;
        r->lo = n.lo % d.lo;
        return;
    }
    dodiv(n, d, 0, r);
}
/*e: function _modvu (arm) */

/*s: function vneg (arm) */
static void
vneg(Vlong *v)
{

    if(v->lo == 0) {
        v->hi = -v->hi;
        return;
    }
    v->lo = -v->lo;
    v->hi = ~v->hi;
}
/*e: function vneg (arm) */

/*s: function _divv (arm) */
void
_divv(Vlong *q, Vlong n, Vlong d)
{
    long nneg, dneg;

    if(n.hi == (((long)n.lo)>>31) && d.hi == (((long)d.lo)>>31)) {
        q->lo = (long)n.lo / (long)d.lo;
        q->hi = ((long)q->lo) >> 31;
        return;
    }
    nneg = n.hi >> 31;
    if(nneg)
        vneg(&n);
    dneg = d.hi >> 31;
    if(dneg)
        vneg(&d);
    dodiv(n, d, q, 0);
    if(nneg != dneg)
        vneg(q);
}
/*e: function _divv (arm) */

/*s: function _modv (arm) */
void
_modv(Vlong *r, Vlong n, Vlong d)
{
    long nneg, dneg;

    if(n.hi == (((long)n.lo)>>31) && d.hi == (((long)d.lo)>>31)) {
        r->lo = (long)n.lo % (long)d.lo;
        r->hi = ((long)r->lo) >> 31;
        return;
    }
    nneg = n.hi >> 31;
    if(nneg)
        vneg(&n);
    dneg = d.hi >> 31;
    if(dneg)
        vneg(&d);
    dodiv(n, d, 0, r);
    if(nneg)
        vneg(r);
}
/*e: function _modv (arm) */

/*s: function _rshav (arm) */
void
_rshav(Vlong *r, Vlong a, int b)
{
    long t;

    t = a.hi;
    if(b >= 32) {
        r->hi = t>>31;
        if(b >= 64) {
            /* this is illegal re C standard */
            r->lo = t>>31;
            return;
        }
        r->lo = t >> (b-32);
        return;
    }
    if(b <= 0) {
        r->hi = t;
        r->lo = a.lo;
        return;
    }
    r->hi = t >> b;
    r->lo = (t << (32-b)) | (a.lo >> b);
}
/*e: function _rshav (arm) */

/*s: function _rshlv (arm) */
void
_rshlv(Vlong *r, Vlong a, int b)
{
    ulong t;

    t = a.hi;
    if(b >= 32) {
        r->hi = 0;
        if(b >= 64) {
            /* this is illegal re C standard */
            r->lo = 0;
            return;
        }
        r->lo = t >> (b-32);
        return;
    }
    if(b <= 0) {
        r->hi = t;
        r->lo = a.lo;
        return;
    }
    r->hi = t >> b;
    r->lo = (t << (32-b)) | (a.lo >> b);
}
/*e: function _rshlv (arm) */

/*s: function _lshv (arm) */
void
_lshv(Vlong *r, Vlong a, int b)
{
    ulong t;

    t = a.lo;
    if(b >= 32) {
        r->lo = 0;
        if(b >= 64) {
            /* this is illegal re C standard */
            r->hi = 0;
            return;
        }
        r->hi = t << (b-32);
        return;
    }
    if(b <= 0) {
        r->lo = t;
        r->hi = a.hi;
        return;
    }
    r->lo = t << b;
    r->hi = (t >> (32-b)) | (a.hi << b);
}
/*e: function _lshv (arm) */

/*s: function _andv (arm) */
void
_andv(Vlong *r, Vlong a, Vlong b)
{
    r->hi = a.hi & b.hi;
    r->lo = a.lo & b.lo;
}
/*e: function _andv (arm) */

/*s: function _orv (arm) */
void
_orv(Vlong *r, Vlong a, Vlong b)
{
    r->hi = a.hi | b.hi;
    r->lo = a.lo | b.lo;
}
/*e: function _orv (arm) */

/*s: function _xorv (arm) */
void
_xorv(Vlong *r, Vlong a, Vlong b)
{
    r->hi = a.hi ^ b.hi;
    r->lo = a.lo ^ b.lo;
}
/*e: function _xorv (arm) */

/*s: function _vpp (arm) */
void
_vpp(Vlong *l, Vlong *r)
{

    l->hi = r->hi;
    l->lo = r->lo;
    r->lo++;
    if(r->lo == 0)
        r->hi++;
}
/*e: function _vpp (arm) */

/*s: function _vmm (arm) */
void
_vmm(Vlong *l, Vlong *r)
{

    l->hi = r->hi;
    l->lo = r->lo;
    if(r->lo == 0)
        r->hi--;
    r->lo--;
}
/*e: function _vmm (arm) */

/*s: function _ppv (arm) */
void
_ppv(Vlong *l, Vlong *r)
{

    r->lo++;
    if(r->lo == 0)
        r->hi++;
    l->hi = r->hi;
    l->lo = r->lo;
}
/*e: function _ppv (arm) */

/*s: function _mmv (arm) */
void
_mmv(Vlong *l, Vlong *r)
{

    if(r->lo == 0)
        r->hi--;
    r->lo--;
    l->hi = r->hi;
    l->lo = r->lo;
}
/*e: function _mmv (arm) */

/*s: function _vasop (arm) */
void
_vasop(Vlong *ret, void *lv, void fn(Vlong*, Vlong, Vlong), int type, Vlong rv)
{
    Vlong t, u;

    u = *ret;
    switch(type) {
    default:
        abort();
        break;

    case 1: /* schar */
        t.lo = *(schar*)lv;
        t.hi = t.lo >> 31;
        fn(&u, t, rv);
        *(schar*)lv = u.lo;
        break;

    case 2: /* uchar */
        t.lo = *(uchar*)lv;
        t.hi = 0;
        fn(&u, t, rv);
        *(uchar*)lv = u.lo;
        break;

    case 3: /* short */
        t.lo = *(short*)lv;
        t.hi = t.lo >> 31;
        fn(&u, t, rv);
        *(short*)lv = u.lo;
        break;

    case 4: /* ushort */
        t.lo = *(ushort*)lv;
        t.hi = 0;
        fn(&u, t, rv);
        *(ushort*)lv = u.lo;
        break;

    case 9: /* int */
        t.lo = *(int*)lv;
        t.hi = t.lo >> 31;
        fn(&u, t, rv);
        *(int*)lv = u.lo;
        break;

    case 10:    /* uint */
        t.lo = *(uint*)lv;
        t.hi = 0;
        fn(&u, t, rv);
        *(uint*)lv = u.lo;
        break;

    case 5: /* long */
        t.lo = *(long*)lv;
        t.hi = t.lo >> 31;
        fn(&u, t, rv);
        *(long*)lv = u.lo;
        break;

    case 6: /* ulong */
        t.lo = *(ulong*)lv;
        t.hi = 0;
        fn(&u, t, rv);
        *(ulong*)lv = u.lo;
        break;

    case 7: /* vlong */
    case 8: /* uvlong */
        fn(&u, *(Vlong*)lv, rv);
        *(Vlong*)lv = u;
        break;
    }
    *ret = u;
}
/*e: function _vasop (arm) */

/*s: function _p2v (arm) */
void
_p2v(Vlong *ret, void *p)
{
    long t;

    t = (ulong)p;
    ret->lo = t;
    ret->hi = 0;
}
/*e: function _p2v (arm) */

/*s: function _sl2v (arm) */
void
_sl2v(Vlong *ret, long sl)
{
    long t;

    t = sl;
    ret->lo = t;
    ret->hi = t >> 31;
}
/*e: function _sl2v (arm) */


/*s: function _ul2v (arm) */
void
_ul2v(Vlong *ret, ulong ul)
{
    long t;

    t = ul;
    ret->lo = t;
    ret->hi = 0;
}
/*e: function _ul2v (arm) */

/*s: function _si2v (arm) */
void
_si2v(Vlong *ret, int si)
{
    long t;

    t = si;
    ret->lo = t;
    ret->hi = t >> 31;
}
/*e: function _si2v (arm) */

/*s: function _ui2v (arm) */
void
_ui2v(Vlong *ret, uint ui)
{
    long t;

    t = ui;
    ret->lo = t;
    ret->hi = 0;
}
/*e: function _ui2v (arm) */

/*s: function _sh2v (arm) */
void
_sh2v(Vlong *ret, long sh)
{
    long t;

    t = (sh << 16) >> 16;
    ret->lo = t;
    ret->hi = t >> 31;
}
/*e: function _sh2v (arm) */

/*s: function _uh2v (arm) */
void
_uh2v(Vlong *ret, ulong ul)
{
    long t;

    t = ul & 0xffff;
    ret->lo = t;
    ret->hi = 0;
}
/*e: function _uh2v (arm) */

/*s: function _sc2v (arm) */
void
_sc2v(Vlong *ret, long uc)
{
    long t;

    t = (uc << 24) >> 24;
    ret->lo = t;
    ret->hi = t >> 31;
}
/*e: function _sc2v (arm) */

/*s: function _uc2v (arm) */
void
_uc2v(Vlong *ret, ulong ul)
{
    long t;

    t = ul & 0xff;
    ret->lo = t;
    ret->hi = 0;
}
/*e: function _uc2v (arm) */

/*s: function _v2sc (arm) */
long
_v2sc(Vlong rv)
{
    long t;

    t = rv.lo & 0xff;
    return (t << 24) >> 24;
}
/*e: function _v2sc (arm) */

/*s: function _v2uc (arm) */
long
_v2uc(Vlong rv)
{

    return rv.lo & 0xff;
}
/*e: function _v2uc (arm) */

/*s: function _v2sh (arm) */
long
_v2sh(Vlong rv)
{
    long t;

    t = rv.lo & 0xffff;
    return (t << 16) >> 16;
}
/*e: function _v2sh (arm) */

/*s: function _v2uh (arm) */
long
_v2uh(Vlong rv)
{

    return rv.lo & 0xffff;
}
/*e: function _v2uh (arm) */

/*s: function _v2sl (arm) */
long
_v2sl(Vlong rv)
{

    return rv.lo;
}
/*e: function _v2sl (arm) */

/*s: function _v2ul (arm) */
long
_v2ul(Vlong rv)
{

    return rv.lo;
}
/*e: function _v2ul (arm) */

/*s: function _v2si (arm) */
long
_v2si(Vlong rv)
{

    return rv.lo;
}
/*e: function _v2si (arm) */

/*s: function _v2ui (arm) */
long
_v2ui(Vlong rv)
{

    return rv.lo;
}
/*e: function _v2ui (arm) */

/*s: function _testv (arm) */
int
_testv(Vlong rv)
{
    return rv.lo || rv.hi;
}
/*e: function _testv (arm) */

/*s: function _eqv (arm) */
int
_eqv(Vlong lv, Vlong rv)
{
    return lv.lo == rv.lo && lv.hi == rv.hi;
}
/*e: function _eqv (arm) */

/*s: function _nev (arm) */
int
_nev(Vlong lv, Vlong rv)
{
    return lv.lo != rv.lo || lv.hi != rv.hi;
}
/*e: function _nev (arm) */

/*s: function _ltv (arm) */
int
_ltv(Vlong lv, Vlong rv)
{
    return (long)lv.hi < (long)rv.hi || 
        (lv.hi == rv.hi && lv.lo < rv.lo);
}
/*e: function _ltv (arm) */

/*s: function _lev (arm) */
int
_lev(Vlong lv, Vlong rv)
{
    return (long)lv.hi < (long)rv.hi || 
        (lv.hi == rv.hi && lv.lo <= rv.lo);
}
/*e: function _lev (arm) */

/*s: function _gtv (arm) */
int
_gtv(Vlong lv, Vlong rv)
{
    return (long)lv.hi > (long)rv.hi || 
        (lv.hi == rv.hi && lv.lo > rv.lo);
}
/*e: function _gtv (arm) */

/*s: function _gev (arm) */
int
_gev(Vlong lv, Vlong rv)
{
    return (long)lv.hi > (long)rv.hi || 
        (lv.hi == rv.hi && lv.lo >= rv.lo);
}
/*e: function _gev (arm) */

/*s: function _lov (arm) */
int
_lov(Vlong lv, Vlong rv)
{
    return lv.hi < rv.hi || 
        (lv.hi == rv.hi && lv.lo < rv.lo);
}
/*e: function _lov (arm) */

/*s: function _lsv (arm) */
int
_lsv(Vlong lv, Vlong rv)
{
    return lv.hi < rv.hi || 
        (lv.hi == rv.hi && lv.lo <= rv.lo);
}
/*e: function _lsv (arm) */

/*s: function _hiv (arm) */
int
_hiv(Vlong lv, Vlong rv)
{
    return lv.hi > rv.hi || 
        (lv.hi == rv.hi && lv.lo > rv.lo);
}
/*e: function _hiv (arm) */

/*s: function _hsv (arm) */
int
_hsv(Vlong lv, Vlong rv)
{
    return lv.hi > rv.hi || 
        (lv.hi == rv.hi && lv.lo >= rv.lo);
}
/*e: function _hsv (arm) */
/*e: arm/vlrt.c */
