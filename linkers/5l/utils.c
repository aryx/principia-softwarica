/*s: linkers/5l/utils.c */
#include "l.h"

/*s: function log */
void mylog(char *fmt, ...) {

    va_list arg;

    va_start(arg, fmt);
    Bvprint(&bso, fmt, arg);
    va_end(arg);
    Bflush(&bso);
}
/*e: function log */

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
    h = sbrk(nh);
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

    // h = hashcode(symb, v)
    h = v;
    for(p=symb; *p; p++) {
        c = *p;
        h = h+h+h + c;
    }
    l = (p - symb) + 1;
    h &= 0xffffff;
    h %= NHASH;
    
    // s = lookup((s->name,v), h, hash)
    for(s = hash[h]; s != S; s = s->link)
        if(s->version == v)
            if(memcmp(s->name, symb, l) == 0)
                return s;

    // else
    s = malloc(sizeof(Sym));
    s->name = malloc(l + 1); // +1 again?
    memmove(s->name, symb, l);
    s->type = SNONE;
    s->version = v;
    s->value = 0;
    s->sig = 0;

    // add_hash(s, hash)
    s->link = hash[h];
    hash[h] = s;

    /*s: [[lookup()]] profiling */
    nsymbol++;
    /*e: [[lookup()]] profiling */
    return s;
}
/*e: function lookup */

/*s: constructor prg */
Prog*
prg(void)
{
    Prog *p;

    p = malloc(sizeof(Prog));
    *p = zprg;
    return p;
}
/*e: constructor prg */

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

/*e: linkers/5l/utils.c */
