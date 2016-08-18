/*s: mk/word.c */
#include	"mk.h"

/*s: constructor newword */
Word*
newword(char *s)
{
    Word *w;

    w = (Word *)Malloc(sizeof(Word));
    w->s = strdup(s);
    w->next = nil;
    return w;
}
/*e: constructor newword */


/*s: function wtos */
char *
wtos(Word *w, int sep)
{
    Bufblock *buf;
    char *cp;

    buf = newbuf();
    for(; w; w = w->next){
        for(cp = w->s; *cp; cp++)
            insert(buf, *cp);
        if(w->next)
            insert(buf, sep);
    }
    insert(buf, '\0');

    cp = strdup(buf->start);
    freebuf(buf);
    return cp;
}
/*e: function wtos */

/*s: function wdup */
Word*
wdup(Word *w)
{
    Word *v, *new, *base;

    v = base = nil;
    while(w){
        new = newword(w->s);
        if(v)
            v->next = new;
        else
            base = new;
        v = new;
        w = w->next;
    }
    return base;
}
/*e: function wdup */

/*s: destructor delword */
void
delword(Word *w)
{
    Word *v;

    while(v = w){
        w = w->next;
        if(v->s)
            free(v->s);
        free(v);
    }
}
/*e: destructor delword */

// was in recipe.c before
/*s: function addw */
void
addw(Word *w, char *s)
{
    Word *lw;

    for(lw = w; w = w->next; lw = w){
        if(strcmp(s, w->s) == 0)
            return;
    }
    lw->next = newword(s);
}
/*e: function addw */



/*e: mk/word.c */
