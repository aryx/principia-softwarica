/*s: 5l/hist.c */
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

    /* claude: mallocz (zeroed): addhist only sets s->name below; every
     * other Sym field (value/type/sig/file/subtype/...) is left implicitly
     * zero, which the old bump allocator guaranteed. With direct lib9
     * malloc they would be garbage in the history symbol. */
    s = mallocz(sizeof(Sym), 1);

    u = malloc(sizeof(Auto));
    u->asym = s;
    u->type = type;
    u->aoffset = line;

    //add_list(u, curhist)
    u->link = curhist;
    curhist = u;

    /*s: [[addhist()]] set symbol name to filename using compact encoding */
    /* claude: mallocz (zeroed), not malloc: the loop below fills only
     * name[1 .. 2*histfrogp]; name[0] (a leading byte emitted by
     * putsymb) and the trailing double-NUL terminator are left for the
     * allocator to zero. The original plan9 code relies on malloc
     * returning zeroed memory; lib9's malloc on this host does not, so
     * putsymb() read uninitialized bytes and ran past the buffer,
     * emitting garbage 'z'/'Z' history records that differed run-to-run
     * and between the two lineages (the last arm unstripped mismatch). */
    s->name = mallocz(2*(histfrogp+1) + 1, 1);
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

/*e: 5l/hist.c */
