/*s: lib_graphics/libmemdraw/unload.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>

/*s: function unloadmemimage */
errorneg1
unloadmemimage(Memimage *i, Rectangle r, byte *data, int ndata)
{
    int y, l;
    byte *q;

    /*s: [[unloadmemimage()]] sanity check r */
    if(!rectinrect(r, i->r))
        return ERROR_NEG1;
    /*e: [[unloadmemimage()]] sanity check r */
    l = bytesperline(r, i->depth);
    /*s: [[unloadmemimage()]] sanity check ndata */
    if(ndata < l*Dy(r))
        return ERROR_NEG1;
    /*e: [[unloadmemimage()]] sanity check ndata */
    ndata = l*Dy(r);
    q = byteaddr(i, r.min);
    for(y = r.min.y; y < r.max.y; y++){
        memmove(data, q, l);
        q += i->width * sizeof(ulong);
        data += l;
    }
    return ndata;
}
/*e: function unloadmemimage */
/*e: lib_graphics/libmemdraw/unload.c */
