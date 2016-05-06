/*s: lib_graphics/libmemdraw/transfer.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>

/*s: function loadmemimage */
errorneg1
loadmemimage(Memimage *i, Rectangle r, byte *data, int ndata)
{
    int l;
    int y;
    byte *q;
    int lpart = 0, rpart = 0;
    /*s: [[loadmemimage()]] other locals */
    int mx, m, mr;
    /*e: [[loadmemimage()]] other locals */

    /*s: [[loadmemimage()]] sanity check r */
    if(!rectinrect(r, i->r))
        return ERROR_NEG1;
    /*e: [[loadmemimage()]] sanity check r */
    l = bytesperline(r, i->depth);
    /*s: [[loadmemimage()]] sanity check ndata */
    if(ndata < l*Dy(r))
        return ERROR_NEG1;
    /*e: [[loadmemimage()]] sanity check ndata */
    ndata = l*Dy(r);

    q = byteaddr(i, r.min);

    /*s: [[loadmemimage()]] set mx, lpart, rpart, m for small depth images */
    mx = 7 / i->depth;
    lpart = (r.min.x & mx) * i->depth;
    rpart = (r.max.x & mx) * i->depth;
    m = 0xFF >> lpart;
    /*e: [[loadmemimage()]] set mx, lpart, rpart, m for small depth images */
    /*s: [[loadmemimage()]] if 1 byte per line */
    /* may need to do bit insertion on edges */
    if(l == 1){	/* all in one byte */
        if(rpart)
            m ^= 0xFF >> rpart;
        for(y = r.min.y; y < r.max.y; y++){
            *q ^= (*data ^ *q) & m;
            q += i->width*sizeof(ulong);
            data++;
        }
        return ndata;
    }
    /*e: [[loadmemimage()]] if 1 byte per line */

    if(lpart==0 && rpart==0){	/* easy case */
        for(y = r.min.y; y < r.max.y; y++){
            memmove(q, data, l);
            q += i->width * sizeof(ulong);
            data += l;
        }
        return ndata;
    }
    /*s: [[loadmemimage()]] when small depth images */
    mr = 0xFF ^ (0xFF >> rpart);
    if(lpart!=0 && rpart==0){
        for(y=r.min.y; y<r.max.y; y++){
            *q ^= (*data^*q) & m;
            if(l > 1)
                memmove(q+1, data+1, l-1);
            q += i->width*sizeof(ulong);
            data += l;
        }
        return ndata;
    }
    if(lpart==0 && rpart!=0){
        for(y=r.min.y; y<r.max.y; y++){
            if(l > 1)
                memmove(q, data, l-1);
            q[l-1] ^= (data[l-1]^q[l-1]) & mr;
            q += i->width*sizeof(ulong);
            data += l;
        }
        return ndata;
    }
    for(y=r.min.y; y<r.max.y; y++){
        *q ^= (*data^*q) & m;
        if(l > 2)
            memmove(q+1, data+1, l-2);
        q[l-1] ^= (data[l-1]^q[l-1]) & mr;
        q += i->width*sizeof(ulong);
        data += l;
    }
    return ndata;
    /*e: [[loadmemimage()]] when small depth images */
}
/*e: function loadmemimage */

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
/*e: lib_graphics/libmemdraw/transfer.c */
