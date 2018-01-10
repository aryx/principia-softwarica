/*s: assemblers/aa/lookup.c */
#include "aa.h"

// syminit() in lookup() depends on LNAME token defined in a.y,
// so it can't be defined here.

/*s: function slookup */
Sym*
slookup(char *s)
{

    strcpy(symb, s);
    return lookup();
}
/*e: function slookup */

/*s: function lookup */
Sym*
lookup(void)
{
    Sym *sym;
    long h;
    int c;
    int len;
    /*s: [[lookup()]] other locals */
    char *p;
    /*e: [[lookup()]] other locals */

    /*s: [[lookup()]] compute hash value [[h]] of [[symb]] */
    // h = hashcode(symb); len = strlen(symb)
    h = 0;
    for(p=symb; c = *p; p++)
        h = h+h+h + c;
    len = (p - symb) + 1;
    if(h < 0)
        h = ~h;
    h %= NHASH;
    /*e: [[lookup()]] compute hash value [[h]] of [[symb]] */

    // hash_lookup(symb, hash)
    c = symb[0];
    for(sym = hash[h]; sym != S; sym = sym->link) {
        // fast path
        if(sym->name[0] != c)
            continue;
        // slow path
        if(memcmp(sym->name, symb, len) == 0)
            return sym;
    }
    /*s: [[lookup()]] if symbol name not found */
    sym = alloc(sizeof(Sym));
    sym->name = alloc(len);
    memmove(sym->name, symb, len);

    // add_hash(sym, hash)
    sym->link = hash[h];
    hash[h] = sym;

    syminit(sym);
    return sym;
    /*e: [[lookup()]] if symbol name not found */
}
/*e: function lookup */
/*e: assemblers/aa/lookup.c */
