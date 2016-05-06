/*s: lib_graphics/libmemdraw/init.c */
#include <u.h>
#include <libc.h>
#include <draw.h>
#include <memdraw.h>
#include <pool.h>

extern Pool* imagmem;

/*s: global memones */
static Memimage*	memones;
/*e: global memones */
/*s: global memzeros */
static Memimage*	memzeros;
/*e: global memzeros */
/*s: global memwhite */
Memimage *memwhite;
/*e: global memwhite */
/*s: global memblack */
Memimage *memblack;
/*e: global memblack */
/*s: global memtransparent */
Memimage *memtransparent;
/*e: global memtransparent */
/*s: global memopaque */
Memimage *memopaque;
/*e: global memopaque */

int	_ifmt(Fmt*);

extern int replmul[];

extern uchar replbit[1+8][256];

/*s: function mktables */
static void
mktables(void)
{
    int i, j, small;
    /*s: [[mktables()]] only once guard */
    static bool	tablesbuilt = false;
    if(tablesbuilt)
        return;
    tablesbuilt = true;
    /*e: [[mktables()]] only once guard */

    /* bit replication up to 8 bits */
    for(i=0; i<256; i++){
        for(j=0; j<=8; j++){	/* j <= 8 [sic] */
            small = i & ((1<<j)-1);
            replbit[j][i] = (small*replmul[j])>>8;
        }
    }

}
/*e: function mktables */


/*s: function memimageinit */
void
memimageinit(void)
{
    /*s: [[memimageinit()]] only once guard */
    static bool didinit = false;
    if(didinit)
        return;
    didinit = true;
    /*e: [[memimageinit()]] only once guard */

    /*s: [[memimageinit()]] set image pool allocator move */
    if(  strcmp(imagmem->name, "Image") == 0 
      || strcmp(imagmem->name, "image") == 0
      )
        imagmem->move = memimagemove;
    /*e: [[memimageinit()]] set image pool allocator move */
    /*s: [[memimageinit()]] initializations */
    _memmkcmap();
    /*x: [[memimageinit()]] initializations */
    mktables();
    /*e: [[memimageinit()]] initializations */
    /*s: [[memimageinit()]] install dumpers */
    fmtinstall('P', Pfmt);
    fmtinstall('R', Rfmt); 
    /*x: [[memimageinit()]] install dumpers */
    fmtinstall('b', _ifmt);
    /*e: [[memimageinit()]] install dumpers */

    memzeros = allocmemimage(Rect(0,0,1,1), GREY1);
    memzeros->flags |= Frepl;
    memzeros->clipr = Rect(-0x3FFFFFF, -0x3FFFFFF, 0x3FFFFFF, 0x3FFFFFF);
    *byteaddr(memzeros, ZP) = 0;

    memones = allocmemimage(Rect(0,0,1,1), GREY1);
    memones->flags |= Frepl;
    memones->clipr = Rect(-0x3FFFFFF, -0x3FFFFFF, 0x3FFFFFF, 0x3FFFFFF);
    *byteaddr(memones, ZP) = ~0;

    /*s: [[memimageinit()]] sanity check memxxx */
    if(memones == nil || memzeros == nil)
        assert(0 /*cannot initialize memimage library */);	/* RSC BUG */
    /*e: [[memimageinit()]] sanity check memxxx */

    memwhite = memones;
    memblack = memzeros;
    memopaque = memones;
    memtransparent = memzeros;
}
/*e: function memimageinit */

/*e: lib_graphics/libmemdraw/init.c */
