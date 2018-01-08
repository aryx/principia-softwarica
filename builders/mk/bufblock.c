/*s: mk/bufblock.c */
#include	"mk.h"

/*s: global [[freelist]] */
static Bufblock *freelist;
/*e: global [[freelist]] */
/*s: constant [[QUANTA]] */
#define	QUANTA	4096
/*e: constant [[QUANTA]] */

/*s: constructor [[newbuf]] */
Bufblock *
newbuf(void)
{
    Bufblock *p;

    if (freelist) {
        p = freelist;
        freelist = freelist->next;
    } else {
        p = (Bufblock *) Malloc(sizeof(Bufblock));
        p->start = Malloc(QUANTA*sizeof(char));
        p->end = p->start+QUANTA;
    }
    p->current = p->start;
    *p->start = '\0';
    p->next = nil;

    return p;
}
/*e: constructor [[newbuf]] */

/*s: destructor [[freebuf]] */
void
freebuf(Bufblock *p)
{
    p->next = freelist;
    freelist = p;
}
/*e: destructor [[freebuf]] */

/*s: function [[growbuf]] */
void
growbuf(Bufblock *p)
{
    int n;
    Bufblock *f;
    char *cp;

    n = p->end-p->start+QUANTA;
        /* search the free list for a big buffer */
    for (f = freelist; f; f = f->next) {
        if (f->end-f->start >= n) {
            memcpy(f->start, p->start, p->end-p->start);
            cp = f->start;
            f->start = p->start;
            p->start = cp;
            cp = f->end;
            f->end = p->end;
            p->end = cp;
            f->current = f->start;
            break;
        }
    }
    if (!f) {		/* not found - grow it */
        p->start = Realloc(p->start, n);
        p->end = p->start+n;
    }
    p->current = p->start+n-QUANTA;
}
/*e: function [[growbuf]] */

/*s: function [[bufcpy]] */
void
bufcpy(Bufblock *buf, char *cp, int n)
{

    while (n--)
        insert(buf, *cp++);
}
/*e: function [[bufcpy]] */

/*s: function [[insert]] */
void
insert(Bufblock *buf, int c)
{

    if (buf->current >= buf->end)
        growbuf(buf);
    *buf->current++ = c;
}
/*e: function [[insert]] */

/*s: function [[rinsert]] */
void
rinsert(Bufblock *buf, Rune r)
{
    int n;

    n = runelen(r);
    if (buf->current+n > buf->end)
        growbuf(buf);
    runetochar(buf->current, &r);
    buf->current += n;
}
/*e: function [[rinsert]] */
/*e: mk/bufblock.c */
