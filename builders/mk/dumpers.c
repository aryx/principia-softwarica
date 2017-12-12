/*s: mk/dumpers.c */
#include	"mk.h"

void	dumpa(char*, Arc*);

/*s: dumper dumpn */
void
dumpn(char *s, Node *n)
{
    char buf[1024];
    Arc *a;

    Bprint(&bout, "%s%s@%p: time=%ld flags=0x%x next=%p\n",
        s, n->name, n, n->time, n->flags, n->next);
    for(a = n->arcs; a; a = a->next){
        snprint(buf, sizeof buf, "%s   ", (*s == ' ')? s:"");
        dumpa(buf, a);
    }
}
/*e: dumper dumpn */

/*s: dumper dumpa */
void
dumpa(char *s, Arc *a)
{
    char buf[1024];

    Bprint(&bout, "%sArc@%p: n=%p r=%p flag=0x%x stem='%s'",
        s, a, a->n, a->r, a->remove, a->stem);
    if(a->prog)
        Bprint(&bout, " prog='%s'", a->prog);
    Bprint(&bout, "\n");

    if(a->n){
        snprint(buf, sizeof(buf), "%s    ", (*s == ' ')? s:"");
        dumpn(buf, a->n);
    }
}
/*e: dumper dumpa */

/*s: dumper dumpj */
void
dumpj(char *s, Job *j, int all)
{
    Bprint(&bout, "%s\n", s);
    while(j){
        Bprint(&bout, "job@%p: r=%p n=%p stem='%s'\n",
            j, j->r, j->n, j->stem);
        Bprint(&bout, "\ttarget='%s' alltarget='%s' prereq='%s' nprereq='%s'\n",
            wtos(j->t, ' '), wtos(j->at, ' '), wtos(j->p, ' '), wtos(j->np, ' '));
        j = all? j->next : nil;
    }
}
/*e: dumper dumpj */

/*s: function [[print1]] */
static void
print1(Symtab *s)
{
    Word *w;

    Bprint(&bout, "\t%s=", s->name);
    for (w = s->u.ptr; w; w = w->next)
        Bprint(&bout, "'%s'", w->s);
    Bprint(&bout, "\n");
}
/*e: function [[print1]] */

/*s: dumper dumpv */
void
dumpv(char *s)
{
    Bprint(&bout, "%s:\n", s);
    symtraverse(S_VAR, &print1);
}
/*e: dumper dumpv */

/*s: dumper dumpr */
void
dumpr(char *s, Rule *r)
{
    Bprint(&bout, "%s: start=%p\n", s, r);
    for(; r; r = r->next){
        Bprint(&bout, "\tRule %p: %s:%d attr=%x next=%p chain=%p alltarget='%s'",
            r, r->file, r->line, r->attr, r->next, r->chain, wtos(r->alltargets, ' '));
        if(r->prog)
            Bprint(&bout, " prog='%s'", r->prog);
        Bprint(&bout, "\n\ttarget=%s: %s\n", r->target, wtos(r->prereqs,' '));
        Bprint(&bout, "\trecipe@%p='%s'\n", r->recipe, r->recipe);
    }
}
/*e: dumper dumpr */

/*s: dumper dumpw */
void
dumpw(char *s, Word *w)
{
    Bprint(&bout, "%s", s);
    for(; w; w = w->next)
        Bprint(&bout, " '%s'", w->s);
    Bputc(&bout, '\n');
}
/*e: dumper dumpw */

/*e: mk/dumpers.c */
