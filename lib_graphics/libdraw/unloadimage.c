/*s: lib_graphics/libdraw/unloadimage.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <draw_private.h>

/*s: function unloadimage */
errorneg1
unloadimage(Image *i, Rectangle r, byte *data, int ndata)
{
    int bpl, dy;
    int n, ntot;
    Display *d;
    /*s: [[unloadimage()]] other locals */
    byte *a;
    /*e: [[unloadimage()]] other locals */

    /*s: [[unloadimage()]] sanity check r */
    if(!rectinrect(r, i->r)){
        werrstr("unloadimage: bad rectangle");
        return ERROR_NEG1;
    }
    /*e: [[unloadimage()]] sanity check r */
    bpl = bytesperline(r, i->depth);
    /*s: [[unloadimage()]] sanity check ndata */
    if(ndata < bpl*Dy(r)){
        werrstr("unloadimage: buffer too small");
        return ERROR_NEG1;
    }
    /*e: [[unloadimage()]] sanity check ndata */

    d = i->display;
    flushimage(d, false);	/* make sure subsequent flush is for us only */

    ntot = 0;
    while(r.min.y < r.max.y){

        dy = 8000/bpl;
        /*s: [[unloadimage()]] sanity check dy */
        if(dy <= 0){
            werrstr("unloadimage: image too wide");
            return ERROR_NEG1;
        }
        /*e: [[unloadimage()]] sanity check dy */
        if(dy > Dy(r))
            dy = Dy(r);

       /*s: [[unloadimage()]] marshall reading request for rectangle of height dy */
       // read: 'r' id[4] R[4*4]
       a = bufimage(d, 1+4+4*4);
       /*s: [[unloadimage()]] sanity check a */
       if(a == nil){
           werrstr("unloadimage: %r");
           return -1;
       }
       /*e: [[unloadimage()]] sanity check a */
       a[0] = 'r';
       BPLONG(a+1, i->id);
       BPLONG(a+5, r.min.x);
       BPLONG(a+9, r.min.y);
       BPLONG(a+13, r.max.x);
       BPLONG(a+17, r.min.y+dy);
       /*e: [[unloadimage()]] marshall reading request for rectangle of height dy */

        n = read(d->fd, data + ntot, ndata - ntot);
       /*s: [[unloadimage()]] sanity check n */
       if(n < 0)
           return n;
       /*e: [[unloadimage()]] sanity check n */
        ntot += n;
        r.min.y += dy; // progress
    }
    return ntot;
}
/*e: function unloadimage */
/*e: lib_graphics/libdraw/unloadimage.c */
