/*s: lib_graphics/libmemdraw/tests/arctest.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>
#include <memlayer.h>

/*s: function [[main]]([[(lib_graphics/libmemdraw/tests/arctest.c)]]) */
void
main(int argc, char **argv)
{
    Memimage *x;
    Point c = {208,871};
    int a = 441;
    int b = 441;
    int thick = 0;
    Point sp = {0,0};
    int alpha = 51;
    int phi = 3;
    vlong t0, t1;
    int i, n;
    vlong del;

    if (argc != 2) {
        fprint(2, "usage: arctest number\n");
        exits("usage");
    }
    memimageinit();

    x = allocmemimage(Rect(0,0,1000,1000), CMAP8);
    n = atoi(argv[1]);

    t0 = nsec();
    t0 = nsec();
    t0 = nsec();
    t1 = nsec();
    del = t1-t0;
    t0 = nsec();
    for(i=0; i<n; i++)
        memarc(x, c, a, b, thick, memblack, sp, alpha, phi, SoverD);
    t1 = nsec();
    print("%lld %lld\n", t1-t0-del, del);
}
/*e: function [[main]]([[(lib_graphics/libmemdraw/tests/arctest.c)]]) */
/*e: lib_graphics/libmemdraw/tests/arctest.c */
