/*s: fmt/fmtrune.c */
#include <u.h>
#include <libc.h>
#include "fmtdef.h"

/*s: function fmtrune */
int
fmtrune(Fmt *f, int r)
{
    Rune *rt;
    char *t;
    int n;

    if(f->runes){
        rt = f->to;
        FMTRCHAR(f, rt, f->stop, r);
        f->to = rt;
        n = 1;
    }else{
        t = f->to;
        FMTRUNE(f, t, f->stop, r);
        n = t - (char*)f->to;
        f->to = t;
    }
    f->nfmt += n;
    return 0;
}
/*e: function fmtrune */
/*e: fmt/fmtrune.c */
