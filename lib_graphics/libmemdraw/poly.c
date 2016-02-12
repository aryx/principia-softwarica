/*s: lib_graphics/libmemdraw/poly.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>
#include <memlayer.h>

/*s: function mempoly */
void
mempoly(Memimage *dst, Point *vert, int nvert, int end0, int end1, int radius, Memimage *src, Point sp, int op)
{
    int i;
    //enum<EndLine>>
    int e0, e1;
    Point d;

    /*s: [[mempoly() sanity check nvert */
    if(nvert < 2)
        return;
    /*e: [[mempoly() sanity check nvert */
    d = subpt(sp, vert[0]); // to compensate for addpt below
    for(i=1; i<nvert; i++){
        /*s: [[mempoly()]] set endlines e0 and e1 */
        e0 = e1 = Enddisc; // for smooth junctions
        if(i == 1)
            e0 = end0;
        if(i == nvert-1)
            e1 = end1;
        /*e: [[mempoly()]] set endlines e0 and e1 */
        memline(dst, vert[i-1], vert[i], e0, e1, radius, src, addpt(d, vert[i-1]), op);
    }
}
/*e: function mempoly */
/*e: lib_graphics/libmemdraw/poly.c */
