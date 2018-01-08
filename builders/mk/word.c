/*s: mk/word.c */
#include	"mk.h"

/*s: constructor [[newword]] */
Word*
newword(char *s)
{
    Word *w;

    w = (Word *)Malloc(sizeof(Word));
    w->s = strdup(s);
    w->next = nil;
    return w;
}
/*e: constructor [[newword]] */


/*s: function [[wtos]] */
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
/*e: function [[wtos]] */

/*s: function [[wdup]] */
Word*
wdup(Word *w)
{
    Word *lastw, *new, *head;

    head = lastw = nil;
    while(w){
        new = newword(w->s);
        if(lastw)
            lastw->next = new;
        else
            head = new;
        lastw = new;
        w = w->next;
    }
    return head;
}
/*e: function [[wdup]] */

/*s: destructor [[freewords]] */
void
freewords(Word *w)
{
    Word *v;

    while(v = w){
        w = w->next;
        if(v->s)
            free(v->s);
        free(v);
    }
}
/*e: destructor [[freewords]] */

// was in recipe.c before
/*s: function [[addw]] */
void
addw(Word *w, char *s)
{
    Word *lastw;

    for(lastw = w; w = w->next; lastw = w){
        if(strcmp(s, w->s) == 0)
            return;
    }
    lastw->next = newword(s);
}
/*e: function [[addw]] */

/*e: mk/word.c */
