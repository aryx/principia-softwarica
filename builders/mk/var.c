/*s: mk/var.c */
#include	"mk.h"

/*s: function setvar */
void
setvar(char *name, void *value)
{
    symlook(name, S_VAR, value)->u.ptr = value;
}
/*e: function setvar */

/*s: function print1 */
static void
print1(Symtab *s)
{
    Word *w;

    Bprint(&bout, "\t%s=", s->name);
    for (w = s->u.ptr; w; w = w->next)
        Bprint(&bout, "'%s'", w->s);
    Bprint(&bout, "\n");
}
/*e: function print1 */

/*s: dumper dumpv */
void
dumpv(char *s)
{
    Bprint(&bout, "%s:\n", s);
    symtraverse(S_VAR, print1);
}
/*e: dumper dumpv */

/*s: function shname */
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
/*e: function shname */
/*e: mk/var.c */
