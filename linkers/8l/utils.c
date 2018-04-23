/*s: linkers/8l/utils.c */
#include "l.h"

/*s: function [[log]] */
void mylog(char *fmt, ...) {

    va_list arg;

    va_start(arg, fmt);
    Bvprint(&bso, fmt, arg);
    va_end(arg);
    Bflush(&bso);
}
/*e: function [[log]] */

/*s: function [[errorexit]] */
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
/*e: function [[errorexit]] */

/*s: function [[gethunk]] */
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
/*e: function [[gethunk]] */

#undef sym // ugly hack for x86
/*s: function [[lookup]] */
Sym*
lookup(char *symb, int v)
{
    Sym *sym;
    long h;
    int len;
    /*s: [[lookup()]] other locals */
    char *p;
    int c;
    /*e: [[lookup()]] other locals */

    /*s: [[lookup()]] compute hash value [[h]] of [[(symb, v)]] and [[len]] */
    // h = hashcode(symb, v); 
    // len = strlen(symb);
    h = v;
    for(p=symb; *p; p++) {
        c = *p;
        h = h+h+h + c;
    }
    len = (p - symb) + 1;
    h &= 0xffffff;
    h %= NHASH;
    /*e: [[lookup()]] compute hash value [[h]] of [[(symb, v)]] and [[len]] */
    
    // sym = hash_lookup((symb, v), h, hash)
    for(sym = hash[h]; sym != S; sym = sym->link)
        if(sym->version == v)
            if(memcmp(sym->name, symb, len) == 0)
                return sym;

    // else
    /*s: [[lookup()]] if symbol name not found */
    sym = malloc(sizeof(Sym));
    sym->name = malloc(len + 1); // +1 again?
    memmove(sym->name, symb, len);
    sym->version = v;

    sym->value = 0;
    sym->type = SNONE;
    sym->sig = 0;

    // add_hash(sym, hash)
    sym->link = hash[h];
    hash[h] = sym;

    /*s: [[lookup()]] profiling */
    nsymbol++;
    /*e: [[lookup()]] profiling */
    return sym;
    /*e: [[lookup()]] if symbol name not found */
}
/*e: function [[lookup]] */

/*s: constructor [[prg]] */
Prog*
prg(void)
{
    Prog *p;

    p = malloc(sizeof(Prog));
    *p = zprg;
    return p;
}
/*e: constructor [[prg]] */

/*s: function [[copyp]] */
Prog*
copyp(Prog *q)
{
    Prog *p;

    p = prg();
    *p = *q;
    return p;
}
/*e: function [[copyp]] */

/*e: linkers/8l/utils.c */
