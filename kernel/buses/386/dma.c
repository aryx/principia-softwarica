/*s: buses/386/dma.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

/*s: dma.c forward decl(x86) */
typedef struct DMAport  DMAport;
typedef struct DMA  DMA;
typedef struct DMAxfer  DMAxfer;
/*e: dma.c forward decl(x86) */

/*
 *  state of a dma transfer
 */
struct DMAxfer
{
    ulong   bpa;        /* bounce buffer physical address */
    void*   bva;        /* bounce buffer virtual address */
    int blen;       /* bounce buffer length */
    void*   va;     /* virtual address destination/src */
    long    len;        /* bytes to be transferred */
    int isread;
};

/*
 *  the dma controllers.  the first half of this structure specifies
 *  the I/O ports used by the DMA controllers.
 */
struct DMAport
{
    uchar   addr[4];    /* current address (4 channels) */
    uchar   count[4];   /* current count (4 channels) */
    uchar   page[4];    /* page registers (4 channels) */
    uchar   cmd;        /* command status register */
    uchar   req;        /* request registers */
    uchar   sbm;        /* single bit mask register */
    uchar   mode;       /* mode register */
    uchar   cbp;        /* clear byte pointer */
    uchar   mc;     /* master clear */
    uchar   cmask;      /* clear mask register */
    uchar   wam;        /* write all mask register bit */
};

struct DMA
{
    DMAport;
    int shift;
    Lock;
    DMAxfer x[4];
};

DMA dma[2] = {
    { 0x00, 0x02, 0x04, 0x06,
      0x01, 0x03, 0x05, 0x07,
      0x87, 0x83, 0x81, 0x82,
      0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
     0 },

    { 0xc0, 0xc4, 0xc8, 0xcc,
      0xc2, 0xc6, 0xca, 0xce,
      0x8f, 0x8b, 0x89, 0x8a,
      0xd0, 0xd2, 0xd4, 0xd6, 0xd8, 0xda, 0xdc, 0xde,
     1 },
};

extern int i8237dma;
static void* i8237bva[2];
static int i8237used;

/*
 *  DMA must be in the first 16MB.  This gets called early by the
 *  initialisation routines of any devices which require DMA to ensure
 *  the allocated bounce buffers are below the 16MB limit.
 */
void
_i8237alloc(void)
{
    void* bva;

    if(i8237dma <= 0)
        return;
    if(i8237dma > 2)
        i8237dma = 2;

    bva = xspanalloc(64*1024*i8237dma, BY2PG, 64*1024);
    if(bva == nil || PADDR(bva)+64*1024*i8237dma > 16*MB){
        /*
         * This will panic with the current
         * implementation of xspanalloc().
        if(bva != nil)
            xfree(bva);
         */
        return;
    }

    i8237bva[0] = bva;
    if(i8237dma == 2)
        i8237bva[1] = ((uchar*)i8237bva[0])+64*1024;
}

/*
 *  DMA must be in the first 16MB.  This gets called early by the
 *  initialisation routines of any devices which require DMA to ensure
 *  the allocated bounce buffers are below the 16MB limit.
 */
int
dmainit(int chan, int maxtransfer)
{
    DMA *dp;
    DMAxfer *xp;
    static int once;

    if(once == 0){
        if(ioalloc(0x00, 0x10, 0, "dma") < 0
        || ioalloc(0x80, 0x10, 0, "dma") < 0
        || ioalloc(0xd0, 0x10, 0, "dma") < 0)
            panic("dmainit");
        once = 1;
    }

    if(maxtransfer > 64*1024)
        maxtransfer = 64*1024;

    dp = &dma[(chan>>2)&1];
    chan = chan & 3;
    xp = &dp->x[chan];
    if(xp->bva != nil){
        if(xp->blen < maxtransfer)
            return 1;
        return 0;
    }

    if(i8237used >= i8237dma || i8237bva[i8237used] == nil){
        print("no i8237 DMA bounce buffer < 16MB\n");
        return 1;
    }
    xp->bva = i8237bva[i8237used++];
    xp->bpa = PADDR(xp->bva);
    xp->blen = maxtransfer;
    xp->len = 0;
    xp->isread = 0;

    return 0;
}

/*
 *  setup a dma transfer.  if the destination is not in kernel
 *  memory, allocate a page for the transfer.
 *
 *  we assume BIOS has set up the command register before we
 *  are booted.
 *
 *  return the updated transfer length (we can't transfer across 64k
 *  boundaries)
 */
long
dmasetup(int chan, void *va, long len, int isread)
{
    DMA *dp;
    ulong pa;
    uchar mode;
    DMAxfer *xp;

    dp = &dma[(chan>>2)&1];
    chan = chan & 3;
    xp = &dp->x[chan];

    /*
     *  if this isn't kernel memory or crossing 64k boundary or above 16 meg
     *  use the bounce buffer.
     */
    if((ulong)va < KZERO 
    || ((pa=PADDR(va))&0xFFFF0000) != ((pa+len)&0xFFFF0000)
    || pa >= 16*MB){
        if(xp->bva == nil)
            return -1;
        if(len > xp->blen)
            len = xp->blen;
        if(!isread)
            memmove(xp->bva, va, len);
        xp->va = va;
        xp->len = len;
        xp->isread = isread;
        pa = xp->bpa;
    }
    else
        xp->len = 0;

    /*
     * this setup must be atomic
     */
    ilock(dp);
    mode = (isread ? 0x44 : 0x48) | chan;
    outb(dp->mode, mode);   /* single mode dma (give CPU a chance at mem) */
    outb(dp->page[chan], pa>>16);
    outb(dp->cbp, 0);       /* set count & address to their first byte */
    outb(dp->addr[chan], pa>>dp->shift);        /* set address */
    outb(dp->addr[chan], pa>>(8+dp->shift));
    outb(dp->count[chan], (len>>dp->shift)-1);      /* set count */
    outb(dp->count[chan], ((len>>dp->shift)-1)>>8);
    outb(dp->sbm, chan);        /* enable the channel */
    iunlock(dp);

    return len;
}

/*
 *  this must be called after a dma has been completed.
 *
 *  if a page has been allocated for the dma,
 *  copy the data into the actual destination
 *  and free the page.
 */
void
dmaend(int chan)
{
    DMA *dp;
    DMAxfer *xp;

    dp = &dma[(chan>>2)&1];
    chan = chan & 3;

    /*
     *  disable the channel
     */
    ilock(dp);
    outb(dp->sbm, 4|chan);
    iunlock(dp);

    xp = &dp->x[chan];
    if(xp->len == 0 || !xp->isread)
        return;

    /*
     *  copy out of temporary page
     */
    memmove(xp->va, xp->bva, xp->len);
    xp->len = 0;
}

/*e: buses/386/dma.c */
