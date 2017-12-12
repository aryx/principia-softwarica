/*s: mk/var.c */
#include	"mk.h"

/*s: function [[setvar]] */
void
setvar(char *name, void *value)
{
    symlook(name, S_VAR, value)->u.ptr = value;
}
/*e: function [[setvar]] */

/*s: function [[shname]] */
char *
shname(char *a)
{
    Rune r;
    int n;

    while (*a) {
        n = chartorune(&r, a);
        if (!WORDCHR(r))
            break;
        a += n;
    }
    return a;
}
/*e: function [[shname]] */
/*e: mk/var.c */
