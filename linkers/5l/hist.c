/*s: linkers/5l/hist.c */
#include	"l.h"

/*s: function [[addhist]] */
void
addhist(long line, int type)
{
    Auto *u;
    Sym *s;
    /*s: [[addhist()]] other locals */
    int i, j, k;
    /*e: [[addhist()]] other locals */

    s = malloc(sizeof(Sym));

    u = malloc(sizeof(Auto));
    u->asym = s;
    u->type = type;
    u->aoffset = line;

    //add_list(u, curhist)
    u->link = curhist;
    curhist = u;

    /*s: [[addhist()]] set symbol name to filename using compact encoding */
    s->name = malloc(2*(histfrogp+1) + 1);
    j = 1;
    for(i=0; i<histfrogp; i++) {
        k = histfrog[i]->value;
        s->name[j+0] = k>>8;
        s->name[j+1] = k;
        j += 2;
    }
    /*e: [[addhist()]] set symbol name to filename using compact encoding */
}
/*e: function [[addhist]] */

/*s: function [[histtoauto]] */
/// ldobj (case AEND | ATEXT) -> <>
void
histtoauto(void)
{
    Auto *l;

    // append_list(curhist, curauto); curhist = nil;
    while(l = curhist) {
        curhist = l->link;

        l->link = curauto;
        curauto = l;
    }
}
/*e: function [[histtoauto]] */

/*e: linkers/5l/hist.c */
