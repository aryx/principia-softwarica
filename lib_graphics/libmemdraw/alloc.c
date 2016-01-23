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
    if(md->base != from){
        print("compacted data not right: #%p\n", md->base);
        abort();
    }
    md->base = to;

    /* if allocmemimage changes this must change too */
    md->bdata = (uchar*)md->base+sizeof(Memdata*)+sizeof(ulong);
}
/*e: function memimagemove */

/*s: function allocmemimaged */
Memimage*
allocmemimaged(Rectangle r, ulong chan, Memdata *md)
{
    int d;
    ulong l;
    Memimage *i;

    if(Dx(r) <= 0 || Dy(r) <= 0){
        werrstr("bad rectangle %R", r);
        return nil;
    }
    if((d = chantodepth(chan)) == 0) {
        werrstr("bad channel descriptor %.8lux", chan);
        return nil;
    }
    l = wordsperline(r, d);

    i = mallocz(sizeof(Memimage), 1);
    if(i == nil)
        return nil;

    i->data = md;

    i->zero = sizeof(ulong) * l * r.min.y;
    if(r.min.x >= 0)
        i->zero += (r.min.x*d)/8;
    else
        i->zero -= (-r.min.x*d+7)/8;
    i->zero = -i->zero; // ???

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
    uchar *p;
    ulong l, nw;
    Memdata *md;
    Memimage *i;

    if((d = chantodepth(chan)) == 0) {
        werrstr("bad channel descriptor %.8lux", chan);
        return nil;
    }

    l = wordsperline(r, d);
    nw = l*Dy(r);

    md = malloc(sizeof(Memdata));
    if(md == nil)
        return nil;
    md->ref = 1;
    // the big alloc!
    md->base = poolalloc(imagmem, sizeof(Memdata*)+(1+nw)*sizeof(ulong));
    if(md->base == nil){
        free(md);
        return nil;
    }

    p = (uchar*)md->base;
    *(Memdata**)p = md;
    p += sizeof(Memdata*);

    *(ulong*)p = getcallerpc(&r);
    p += sizeof(ulong);

    /* if this changes, memimagemove must change too */
    md->bdata = p;
    md->allocd = true;

    i = allocmemimaged(r, chan, md);
    if(i == nil){
        poolfree(imagmem, md->base);
        free(md);
        return nil;
    }
    md->imref = i;
    return i;
}
/*e: function allocmemimage */

/*s: function freememimage */
void
freememimage(Memimage *i)
{
    if(i == nil)
        return;
    // free the Memdata
    if(i->data->ref-- == 1 && i->data->allocd){
        if(i->data->base)
            poolfree(imagmem, i->data->base);
        free(i->data);
    }
    free(i);
}
/*e: function freememimage */

/*s: function wordaddr */
/*
 * Wordaddr is deprecated.
 */
ulong*
wordaddr(Memimage *i, Point p)
{
    return (ulong*) ((uintptr)byteaddr(i, p) & ~(sizeof(ulong)-1));
}
/*e: function wordaddr */

/*s: function byteaddr */
byte*
byteaddr(Memimage *i, Point p)
{
    byte *a;

    a = (i->data->bdata + i->zero) + (sizeof(ulong) * p.y * i->width);

    /*s: [[byteaddr()]] if depth less than 8 */
    if(i->depth < 8){
        /*
         * We need to always round down,
         * but C rounds toward zero.
         */
        int np;
        np = 8/i->depth;
        if(p.x < 0)
            return a+(p.x-np+1)/np;
        else
            return a+p.x/np;
    }
    /*e: [[byteaddr()]] if depth less than 8 */
    else
        return a + p.x*(i->depth/8);
}
/*e: function byteaddr */

/*s: function memsetchan */
errorneg1
memsetchan(Memimage *i, ulong chan)
{
    int d;
    int t, j, k;
    ulong cc;
    bool bytes;

    if((d = chantodepth(chan)) == 0) {
        werrstr("bad channel descriptor");
        return ERROR_NEG1;
    }

    i->chan = chan;
    i->depth = d;
    i->flags &= ~(Fgrey|Falpha|Fcmap|Fbytes);
    bytes = true;

    for(cc=chan, j=0, k=0; cc; j+=NBITS(cc), cc>>=8, k++){
        t=TYPE(cc);
        if(t < 0 || t >= NChan){
            werrstr("bad channel string");
            return -1;
        }
        if(t == CGrey)
            i->flags |= Fgrey;
        if(t == CAlpha)
            i->flags |= Falpha;
        if(t == CMap && i->cmap == nil){
            i->cmap = memdefcmap;
            i->flags |= Fcmap;
        }

        i->shift[t] = j;
        i->mask[t] = (1<<NBITS(cc))-1;
        i->nbits[t] = NBITS(cc);

        if(NBITS(cc) != 8)
            bytes = false;
    }
    i->nchan = k;
    if(bytes)
        i->flags |= Fbytes;
    return 0;
}
/*e: function memsetchan */
/*e: lib_graphics/libmemdraw/alloc.c */
