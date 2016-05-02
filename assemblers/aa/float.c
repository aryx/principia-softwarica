/*s: assemblers/aa/float.c */
#include "aa.h"

/*s: function ieeedtod */
void
ieeedtod(Ieee *ieee, double native)
{
    double fr, ho, f;
    int exp;

    if(native < 0) {
        ieeedtod(ieee, -native);
        ieee->h |= 0x80000000L;
        return;
    }
    if(native == 0) {
        ieee->l = 0;
        ieee->h = 0;
        return;
    }
    fr = frexp(native, &exp);
    f = 2097152L;		/* shouldnt use fp constants here */
    fr = modf(fr*f, &ho);
    ieee->h = ho;
    ieee->h &= 0xfffffL;
    ieee->h |= (exp+1022L) << 20;
    f = 65536L;
    fr = modf(fr*f, &ho);
    ieee->l = ho;
    ieee->l <<= 16;
    ieee->l |= (long)(fr*f);
}
/*e: function ieeedtod */

/*e: assemblers/aa/float.c */
