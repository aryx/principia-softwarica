/*s: rc/words.c */
/*s: includes */
#include "rc.h"
#include "exec.h"
#include "fns.h"
#include "getflags.h"
#include "io.h"
/*e: includes */

/*s: function [[newword]] */
word*
newword(char *wd, word *next)
{
    word *p = new(struct Word);
    p->word = strdup(wd);
    p->next = next;
    return p;
}
/*e: function [[newword]] */

/*s: function [[freewords]] */
void
freewords(word *w)
{
    word *nw;
    while(w){
        efree(w->word);
        nw = w->next;
        efree((char *)w);
        w = nw;
    }
}
/*e: function [[freewords]] */

/*s: function [[count]] */
int
count(word *w)
{
    int n;
    for(n = 0;w;n++) 
        w = w->next;
    return n;
}
/*e: function [[count]] */

/*s: function [[copynwords]] */
word*
copynwords(word *a, word *tail, int n)
{
    word *v = nil;
    word **end = &v;
    
    while(n-- > 0){
        *end = newword(a->word, 0);
        end = &(*end)->next;
        a = a->next;
    }
    *end = tail;
    return v;
}
/*e: function [[copynwords]] */

/*s: function [[copywords]] */
/*
 * copy arglist a, adding the copy to the front of tail
 */
word*
copywords(word *a, word *tail)
{
    word *v = nil;
    word **end;

    for(end=&v;a;a = a->next,end=&(*end)->next)
        *end = newword(a->word, nil);
    *end = tail;
    return v;
}
/*e: function [[copywords]] */

/*s: function [[freelist]] */
void
freelist(word *w)
{
    word *nw;
    while(w){
        nw = w->next;
        efree(w->word);
        efree((char *)w);
        w = nw;
    }
}
/*e: function [[freelist]] */

/*e: rc/words.c */
