/*s: mk/arc.c */
#include	"mk.h"

/*s: constructor newarc */
Arc*
newarc(Node *n, Rule *r, char *stem, Resub *match)
{
    Arc *a;

    a = (Arc *)Malloc(sizeof(Arc));
    a->n = n;
    a->r = r;
    a->stem = strdup(stem);

    a->next = nil;
    a->flag = 0;
    /*s: [[newarc()]] set other fields */
    rcopy(a->match, match, NREGEXP);
    /*x: [[newarc()]] set other fields */
    a->prog = r->prog;
    /*e: [[newarc()]] set other fields */
    return a;
}
/*e: constructor newarc */

/*s: dumper dumpa */
void
dumpa(char *s, Arc *a)
{
    char buf[1024];

    Bprint(&bout, "%sArc@%p: n=%p r=%p flag=0x%x stem='%s'",
        s, a, a->n, a->r, a->flag, a->stem);
    if(a->prog)
        Bprint(&bout, " prog='%s'", a->prog);
    Bprint(&bout, "\n");

    if(a->n){
        snprint(buf, sizeof(buf), "%s    ", (*s == ' ')? s:"");
        dumpn(buf, a->n);
    }
}
/*e: dumper dumpa */

/*s: function nrep */
void
nrep(void)
{
    Symtab *sym;
    Word *w;

    sym = symlook("NREP", S_VAR, nil);
    if(sym){
        w = sym->u.ptr;
        if (w && w->s && *w->s)
            nreps = atoi(w->s);
    }
    if(nreps < 1)
        nreps = 1;
    /*s: [[nrep()]] if DEBUG(D_GRAPH) */
    if(DEBUG(D_GRAPH))
        Bprint(&bout, "nreps = %d\n", nreps);
    /*e: [[nrep()]] if DEBUG(D_GRAPH) */
}
/*e: function nrep */
/*e: mk/arc.c */
