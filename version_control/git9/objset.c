/*s: git9/objset.c */
#include <u.h>
#include <libc.h>

#include "git.h"

/*s: function [[osinit]] */
void
osinit(Objset *s)
{
    s->sz = 16;
    s->nobj = 0;
    s->obj = eamalloc(s->sz, sizeof(Object*));
}
/*e: function [[osinit]] */
/*s: function [[osclear]] */
void
osclear(Objset *s)
{
    free(s->obj);
}
/*e: function [[osclear]] */

/*s: function [[osadd]] */
void
osadd(Objset *s, Object *o)
{
    u32int probe;
    /*s: [[osadd()]] other locals */
    Object **obj;
    int i, sz;
    /*e: [[osadd()]] other locals */

    probe = GETBE32(o->hash.h) % s->sz;
    while(s->obj[probe]){
        if(hasheq(&s->obj[probe]->hash, &o->hash)){
            // update the object (old = leak?)
            s->obj[probe] = o;
            return;
        }
        // else
        probe = (probe + 1) % s->sz;
    }
    assert(s->obj[probe] == nil);
    s->obj[probe] = o;
    s->nobj++;

    /*s: [[osadd()]] realloc if needed */
    if(s->sz < 2*s->nobj){
        sz = s->sz;
        obj = s->obj;

        s->sz *= 2;
        s->nobj = 0;
        s->obj = eamalloc(s->sz, sizeof(Object*));
        for(i = 0; i < sz; i++)
            if(obj[i])
                // recurse! need to maintain hash invariant
                osadd(s, obj[i]);
        free(obj);
    }
    /*e: [[osadd()]] realloc if needed */
}
/*e: function [[osadd]] */
/*s: function [[osfind]] */
Object*
osfind(Objset *s, Hash h)
{
    u32int probe;

    for(probe = GETBE32(h.h) % s->sz; s->obj[probe]; probe = (probe + 1) % s->sz)
        if(hasheq(&s->obj[probe]->hash, &h))
            return s->obj[probe]; 
    // else
    return nil;
}
/*e: function [[osfind]] */
/*s: function [[oshas]] */
int
oshas(Objset *s, Hash h)
{
    return osfind(s, h) != nil;
}
/*e: function [[oshas]] */
/*e: git9/objset.c */
