/*s: mk/symtab.c */
#include	"mk.h"

/*s: constant [[NHASH]] */
#define	NHASH	4099
/*e: constant [[NHASH]] */
/*s: constant [[HASHMUL]] */
#define	HASHMUL	79L	/* this is a good value */
/*e: constant [[HASHMUL]] */
/*s: global [[hash]] */
// hash<(string * enum<Namespace>), 'a> (next = Symtab.next in bucket)
static Symtab *hash[NHASH];
/*e: global [[hash]] */

/*s: function [[symlook]] */
Symtab*
symlook(char *sym, int space, void *install)
{
    Symtab *s;
    long h;
    /*s: [[symlook()]] other locals */
    char *p;
    /*e: [[symlook()]] other locals */

    /*s: [[symlook()]] compute hash value [[h]] of [[sym]] */
    //h = hash(sym, space)
    for(p = sym, h = space; *p; h += *p++)
        h *= HASHMUL;
    if(h < 0)
        h = ~h;
    h %= NHASH;
    /*e: [[symlook()]] compute hash value [[h]] of [[sym]] */

    // s = hash_lookup((sym, space), h, hash)
    for(s = hash[h]; s; s = s->next)
        if((s->space == space) && (strcmp(s->name, sym) == 0))
            return s;
    // else
    /*s: [[symlook()]] if symbol not found */
    if(install == nil)
        return nil;

    s = (Symtab *)Malloc(sizeof(Symtab));
    s->name = sym;
    s->space = space;
    s->u.ptr = install;

    // add_list(s, hash)
    s->next = hash[h];
    hash[h] = s;

    return s;
    /*e: [[symlook()]] if symbol not found */
}
/*e: function [[symlook]] */

/*s: function [[symtraverse]] */
void
symtraverse(int space, void (*fn)(Symtab*))
{
    Symtab **s, *ss;

    for(s = hash; s < &hash[NHASH]; s++)
        for(ss = *s; ss; ss = ss->next)
            if(ss->space == space)
                (*fn)(ss);
}
/*e: function [[symtraverse]] */

/*s: function [[symstat]] */
void
symstat(void)
{
    Symtab **s, *ss;
    int n;
    int l[1000];

    memset((char *)l, 0, sizeof(l));
    for(s = hash; s < &hash[NHASH]; s++){
        for(ss = *s, n = 0; ss; ss = ss->next)
            n++;
        l[n]++;
    }
    for(n = 0; n < 1000; n++)
        if(l[n]) 
            Bprint(&bout, "%d of length %d\n", l[n], n);
}
/*e: function [[symstat]] */
/*e: mk/symtab.c */
