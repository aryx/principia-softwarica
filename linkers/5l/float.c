/*s: linkers/5l/float.c */
#include	"l.h"

/*s: function [[ieeedtof]] */
/// main -> objfile -> ldobj -> <>
long
ieeedtof(Ieee *e)
{
    int exp;
    long v;

    if(e->h == 0)
        return 0;
    exp = (e->h>>20) & ((1L<<11)-1L);
    exp -= (1L<<10) - 2L;
    v = (e->h & 0xfffffL) << 3;
    v |= (e->l >> 29) & 0x7L;
    if((e->l >> 28) & 1) {
        v++;
        if(v & 0x800000L) {
            v = (v & 0x7fffffL) >> 1;
            exp++;
        }
    }
    if(exp <= -126 || exp >= 130)
        diag("double fp to single fp overflow");
    v |= ((exp + 126) & 0xffL) << 23;
    v |= e->h & 0x80000000L;
    return v;
}
/*e: function [[ieeedtof]] */

/*s: function [[ieeedtod]] */
/// Dconv -> <>
double
ieeedtod(Ieee *ieeep)
{
    Ieee e;
    double fr;
    int exp;

    if(ieeep->h & (1L<<31)) {
        e.h = ieeep->h & ~(1L<<31);
        e.l = ieeep->l;
        return -ieeedtod(&e);
    }
    if(ieeep->l == 0 && ieeep->h == 0)
        return 0;
    fr = ieeep->l & ((1L<<16)-1L);
    fr /= 1L<<16;
    fr += (ieeep->l>>16) & ((1L<<16)-1L);
    fr /= 1L<<16;
    fr += (ieeep->h & (1L<<20)-1L) | (1L<<20);
    fr /= 1L<<21;
    exp = (ieeep->h>>20) & ((1L<<11)-1L);
    exp -= (1L<<10) - 2L;
    return ldexp(fr, exp);
}
/*e: function [[ieeedtod]] */

/*s: global [[chipfloats]](arm) */
static Ieee chipfloats[] = {
    {0x00000000, 0x00000000}, /* 0 */
    {0x00000000, 0x3ff00000}, /* 1 */
    {0x00000000, 0x40000000}, /* 2 */
    {0x00000000, 0x40080000}, /* 3 */
    {0x00000000, 0x40100000}, /* 4 */
    {0x00000000, 0x40140000}, /* 5 */
    {0x00000000, 0x3fe00000}, /* .5 */
    {0x00000000, 0x40240000}, /* 10 */
};
/*e: global [[chipfloats]](arm) */

/*s: function [[chipfloat]](arm) */
int
chipfloat(Ieee *e)
{
    Ieee *p;
    int n;

    if(vfp)
        return -1;
    for(n = sizeof(chipfloats)/sizeof(chipfloats[0]); --n >= 0;){
        p = &chipfloats[n];
        if(p->l == e->l && p->h == e->h)
            return n;
    }
    return -1;
}
/*e: function [[chipfloat]](arm) */

/*e: linkers/5l/float.c */
