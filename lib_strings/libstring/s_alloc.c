/*s: libstring/s_alloc.c */
/*s: libstring includes */
#include <u.h>
#include <libc.h>
#include <str.h> // was string.h
/*e: libstring includes */

/*s: constant [[STRLEN]] */
#define STRLEN 128
/*e: constant [[STRLEN]] */

/*s: function [[s_free]] */
extern void
s_free(String *sp)
{
    if (sp == nil)
        return;
    lock(sp);
    if(--(sp->ref) != 0){
        unlock(sp);
        return;
    }
    unlock(sp);

    if(sp->fixed == 0 && sp->base != nil)
        free(sp->base);
    free(sp);
}
/*e: function [[s_free]] */

/*s: function [[s_incref]] */
/* get another reference to a string */
extern String *
s_incref(String *sp)
{
    lock(sp);
    sp->ref++;
    unlock(sp);

    return sp;
}
/*e: function [[s_incref]] */

/*s: function [[_s_alloc]] */
/* allocate a String head */
extern String *
_s_alloc(void)
{
    String *s;

    s = mallocz(sizeof *s, 1);
    if(s == nil)
        return s;
    s->ref = 1;
    s->fixed = 0;
    return s;
}
/*e: function [[_s_alloc]] */

/*s: function [[s_newalloc]] */
/* create a new `short' String */
extern String *
s_newalloc(int len)
{
    String *sp;

    sp = _s_alloc();
    if(sp == nil)
        sysfatal("s_newalloc: %r");
    setmalloctag(sp, getcallerpc(&len));
    if(len < STRLEN)
        len = STRLEN;
    sp->base = sp->ptr = malloc(len);
    if (sp->base == nil)
        sysfatal("s_newalloc: %r");
    setmalloctag(sp->base, getcallerpc(&len));

    sp->end = sp->base + len;
    s_terminate(sp);
    return sp;
}
/*e: function [[s_newalloc]] */

/*s: function [[s_new]] */
/* create a new `short' String */
extern String *
s_new(void)
{
    String *sp;

    sp = _s_alloc();
    if(sp == nil)
        sysfatal("s_new: %r");
    sp->base = sp->ptr = malloc(STRLEN);
    if (sp->base == nil)
        sysfatal("s_new: %r");
    sp->end = sp->base + STRLEN;
    s_terminate(sp);
    return sp;
}
/*e: function [[s_new]] */
/*e: libstring/s_alloc.c */
