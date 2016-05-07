/*s: lib_graphics/libmemdraw/alloc.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>
#include <pool.h>

/*s: function memimagemove */
void
memimagemove(void *from, void *to)
{
    Memdata *md;

    md = *(Memdata**)to;
    /*s: [[memimagemove()]] sanity check md base */
    if(md->base != from){
        print("compacted data not right: #%p\n", md->base);
        abort();
    }
    /*e: [[memimagemove()]] sanity check md base */
    md->base = to;

    /* if allocmemimage changes this must change too */
    md->bdata = (byte*)md->base + sizeof(Memdata*) + sizeof(ulong);
}
/*e: function memimagemove */

/*s: function allocmemimaged */
Memimage*
allocmemimaged(Rectangle r, ulong chan, Memdata *md)
{
    int d;
    ulong l;
    Memimage *i;

    /*s: [[allocmemimaged()]] sanity check r */
    if(Dx(r) <= 0 || Dy(r) <= 0){
        werrstr("bad rectangle %R", r);
        return nil;
    }
    /*e: [[allocmemimaged()]] sanity check r */
    d = chantodepth(chan);
    /*s: [[allocmemimaged()]] sanity check d */
    if(d == 0) {
        werrstr("bad channel descriptor %.8lux", chan);
        return nil;
    }
    /*e: [[allocmemimaged()]] sanity check d */
    l = wordsperline(r, d);

    i = mallocz(sizeof(Memimage), true);
    /*s: [[allocmemimaged()]] sanity check i */
    if(i == nil)
        return nil;
    /*e: [[allocmemimaged()]] sanity check i */

    i->data = md;
    /*s: [[allocmemimaged()]] set zero field */
    i->zero = sizeof(ulong) * l * r.min.y;
    if(r.min.x >= 0)
        i->zero += (r.min.x*d)/8;
    else
        i->zero -= (-r.min.x*d+7)/8;
    i->zero = -i->zero;
    /*e: [[allocmemimaged()]] set zero field */
    i->width = l;
    i->r = r;
    i->clipr = r;
    i->flags = 0;

    i->layer = nil;
    i->cmap = memdefcmap;

    if(memsetchan(i, chan) < 0){
        free(i);
        return nil;
    }
    return i;
}
/*e: function allocmemimaged */


/*s: function allocmemimage */
Memimage*
allocmemimage(Rectangle r, ulong chan)
{
    int d;
    byte *p;
    ulong l, nw;
    Memdata *md;
    Memimage *i;

    d = chantodepth(chan);
    /*s: [[allocmemimage()]] sanity check d */
    if(d == 0) {
        werrstr("bad channel descriptor %.8lux", chan);
        return nil;
    }
    /*e: [[allocmemimage()]] sanity check d */

    l = wordsperline(r, d);
    nw = l * Dy(r);

    md = malloc(sizeof(Memdata));
    /*s: [[allocmemimage()]] sanity check md */
    if(md == nil)
        return nil;
    /*e: [[allocmemimage()]] sanity check md */
    md->ref = 1;
    // the big alloc!
    md->base = poolalloc(imagmem, sizeof(Memdata*)+(1+nw)*sizeof(ulong));
    /*s: [[allocmemimage()]] sanity check md base */
    if(md->base == nil){
        free(md);
        return nil;
    }
    /*e: [[allocmemimage()]] sanity check md base */

    p = (byte*)md->base;
    *(Memdata**)p = md;
    p += sizeof(Memdata*);
    *(ulong*)p = getcallerpc(&r);
    p += sizeof(ulong);

    /* if this changes, memimagemove must change too */
    md->bdata = p;
    md->allocd = true;

    i = allocmemimaged(r, chan, md);
    /*s: [[allocmemimage()]] sanity check i */
    if(i == nil){
        poolfree(imagmem, md->base);
        free(md);
        return nil;
    }
    /*e: [[allocmemimage()]] sanity check i */

    return i;
}
/*e: function allocmemimage */

/*s: function freememimage */
void
freememimage(Memimage *i)
{
    /*s: [[freememimage()]] sanity check i */
    if(i == nil)
        return;
    /*e: [[freememimage()]] sanity check i */
    // free the Memdata
    if(i->data->ref-- == 1 && i->data->allocd){
        if(i->data->base)
            poolfree(imagmem, i->data->base);
        free(i->data);
    }
    free(i);
}
/*e: function freememimage */

/*e: lib_graphics/libmemdraw/alloc.c */
