/*s: linkers/8l/utils.c */
#include "l.h"

/*s: function errorexit */
void
errorexit(void)
{

    if(nerrors) {
        if(cout >= 0)
            remove(outfile);
        exits("error");
    }
    exits(0);
}
/*e: function errorexit */

/*s: function gethunk */
void
gethunk(void)
{
    char *h;
    long nh;

    nh = NHUNK;
    if(thunk >= 5L*NHUNK) {
        nh = 5L*NHUNK;
        if(thunk >= 25L*NHUNK)
            nh = 25L*NHUNK;
    }
    h = mysbrk(nh);
    if(h == (char*)-1) {
        diag("out of memory");
        errorexit();
    }
    hunk = h;
    nhunk = nh;
    thunk += nh;
}
/*e: function gethunk */

/*s: function lookup */
Sym*
lookup(char *symb, int v)
{
    Sym *s;
    char *p;
    long h;
    int l, c;

    h = v;
    for(p=symb; c = *p; p++)
        h = h+h+h + c;
    l = (p - symb) + 1;
    h &= 0xffffff;
    h %= NHASH;
    for(s = hash[h]; s != S; s = s->link)
        if(s->version == v)
        if(memcmp(s->name, symb, l) == 0)
            return s;

    while(nhunk < sizeof(Sym))
        gethunk();
    s = (Sym*)hunk;
    nhunk -= sizeof(Sym);
    hunk += sizeof(Sym);

    s->name = malloc(l + 1);
    memmove(s->name, symb, l);

    s->link = hash[h];
    s->type = 0;
    s->version = v;
    s->value = 0;
    s->sig = 0;
    hash[h] = s;
    nsymbol++;
    return s;
}
/*e: function lookup */

/*s: function prg */
Prog*
prg(void)
{
    Prog *p;

    while(nhunk < sizeof(Prog))
        gethunk();
    p = (Prog*)hunk;
    nhunk -= sizeof(Prog);
    hunk += sizeof(Prog);

    *p = zprg;
    return p;
}
/*e: function prg */

/*s: function copyp */
Prog*
copyp(Prog *q)
{
    Prog *p;

    p = prg();
    *p = *q;
    return p;
}
/*e: function copyp */


/*e: linkers/8l/utils.c */
