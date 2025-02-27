/*s: kernel/network/386/ether8390.c */
/*
 * National Semiconductor DP8390 and clone
 * Network Interface Controller.
 */
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"
#include "../port/error.h"
#include "../port/netif.h"
#include "../port/etherif.h"

#include "ether8390.h"

/*s: enum [[_anon_ (kernel/network/386/ether8390.c)0]] */
enum {					/* NIC core registers */
    Cr		= 0x00,		/* command register, all pages */

                    /* Page 0, read */
    Clda0		= 0x01,		/* current local DMA address 0 */
    Clda1		= 0x02,		/* current local DMA address 1 */
    Bnry		= 0x03,		/* boundary pointer (R/W) */
    Tsr		= 0x04,		/* transmit status register */
    Ncr		= 0x05,		/* number of collisions register */
    Fifo		= 0x06,		/* FIFO */
    Isr		= 0x07,		/* interrupt status register (R/W) */
    Crda0		= 0x08,		/* current remote DMA address 0 */
    Crda1		= 0x09,		/* current remote DMA address 1 */
    Rsr		= 0x0C,		/* receive status register */
    Ref0		= 0x0D,		/* frame alignment errors */
    Ref1		= 0x0E,		/* CRC errors */
    Ref2		= 0x0F,		/* missed packet errors */

                    /* Page 0, write */
    Pstart		= 0x01,		/* page start register */
    Pstop		= 0x02,		/* page stop register */
    Tpsr		= 0x04,		/* transmit page start address */
    Tbcr0		= 0x05,		/* transmit byte count register 0 */
    Tbcr1		= 0x06,		/* transmit byte count register 1 */
    Rsar0		= 0x08,		/* remote start address register 0 */
    Rsar1		= 0x09,		/* remote start address register 1 */
    Rbcr0		= 0x0A,		/* remote byte count register 0 */
    Rbcr1		= 0x0B,		/* remote byte count register 1 */
    Rcr		= 0x0C,		/* receive configuration register */
    Tcr		= 0x0D,		/* transmit configuration register */
    Dcr		= 0x0E,		/* data configuration register */
    Imr		= 0x0F,		/* interrupt mask */

                    /* Page 1, read/write */
    Par0		= 0x01,		/* physical address register 0 */
    Curr		= 0x07,		/* current page register */
    Mar0		= 0x08,		/* multicast address register 0 */
};
/*e: enum [[_anon_ (kernel/network/386/ether8390.c)0]] */

/*s: enum [[_anon_ (kernel/network/386/ether8390.c)1]] */
enum {					/* Cr */
    Stp		= 0x01,		/* stop */
    Sta		= 0x02,		/* start */
    Txp		= 0x04,		/* transmit packet */
    Rd0		= 0x08,		/* remote DMA command */
    Rd1		= 0x10,	
    Rd2		= 0x20,
    RdREAD		= Rd0,		/* remote read */
    RdWRITE		= Rd1,		/* remote write */
    RdSEND		= Rd1|Rd0,	/* send packet */
    RdABORT		= Rd2,		/* abort/complete remote DMA */
    Ps0		= 0x40,		/* page select */
    Ps1		= 0x80,
    Page0		= 0x00,
    Page1		= Ps0,
    Page2		= Ps1,
};
/*e: enum [[_anon_ (kernel/network/386/ether8390.c)1]] */

/*s: enum [[_anon_ (kernel/network/386/ether8390.c)2]] */
enum {					/* Isr/Imr */
    Prx		= 0x01,		/* packet received */
    Ptx		= 0x02,		/* packet transmitted */
    Rxe		= 0x04,		/* receive error */
    Txe		= 0x08,		/* transmit error */
    Ovw		= 0x10,		/* overwrite warning */
    Cnt		= 0x20,		/* counter overflow */
    Rdc		= 0x40,		/* remote DMA complete */
    Rst		= 0x80,		/* reset status */
};
/*e: enum [[_anon_ (kernel/network/386/ether8390.c)2]] */

/*s: enum [[_anon_ (kernel/network/386/ether8390.c)3]] */
enum {					/* Dcr */
    Wts		= 0x01,		/* word transfer select */
    Bos		= 0x02,		/* byte order select */
    Las		= 0x04,		/* long address select */
    Ls		= 0x08,		/* loopback select */
    Arm		= 0x10,		/* auto-initialise remote */
    Ft0		= 0x20,		/* FIFO threshold select */
    Ft1		= 0x40,
    Ft1WORD		= 0x00,
    Ft2WORD		= Ft0,
    Ft4WORD		= Ft1,
    Ft6WORD		= Ft1|Ft0,
};
/*e: enum [[_anon_ (kernel/network/386/ether8390.c)3]] */

/*s: enum [[_anon_ (kernel/network/386/ether8390.c)4]] */
enum {					/* Tcr */
    Crc		= 0x01,		/* inhibit CRC */
    Lb0		= 0x02,		/* encoded loopback control */
    Lb1		= 0x04,
    LpbkNORMAL	= 0x00,		/* normal operation */
    LpbkNIC		= Lb0,		/* internal NIC module loopback */
    LpbkENDEC	= Lb1,		/* internal ENDEC module loopback */
    LpbkEXTERNAL	= Lb1|Lb0,	/* external loopback */
    Atd		= 0x08,		/* auto transmit disable */
    Ofst		= 0x10,		/* collision offset enable */
};
/*e: enum [[_anon_ (kernel/network/386/ether8390.c)4]] */

/*s: enum [[_anon_ (kernel/network/386/ether8390.c)5]] */
enum {					/* Tsr */
    Ptxok		= 0x01,		/* packet transmitted */
    Col		= 0x04,		/* transmit collided */
    Abt		= 0x08,		/* tranmit aborted */
    Crs		= 0x10,		/* carrier sense lost */
    Fu		= 0x20,		/* FIFO underrun */
    Cdh		= 0x40,		/* CD heartbeat */
    Owc		= 0x80,		/* out of window collision */
};
/*e: enum [[_anon_ (kernel/network/386/ether8390.c)5]] */

/*s: enum [[_anon_ (kernel/network/386/ether8390.c)6]] */
enum {					/* Rcr */
    Sep		= 0x01,		/* save errored packets */
    Ar		= 0x02,		/* accept runt packets */
    Ab		= 0x04,		/* accept broadcast */
    Am		= 0x08,		/* accept multicast */
    Pro		= 0x10,		/* promiscuous physical */
    Mon		= 0x20,		/* monitor mode */
};
/*e: enum [[_anon_ (kernel/network/386/ether8390.c)6]] */

/*s: enum [[_anon_ (kernel/network/386/ether8390.c)7]] */
enum {					/* Rsr */
    Prxok		= 0x01,		/* packet received intact */
    Crce		= 0x02,		/* CRC error */
    Fae		= 0x04,		/* frame alignment error */
    Fo		= 0x08,		/* FIFO overrun */
    Mpa		= 0x10,		/* missed packet */
    Phy		= 0x20,		/* physical/multicast address */
    Dis		= 0x40,		/* receiver disabled */
    Dfr		= 0x80,		/* deferring */
};
/*e: enum [[_anon_ (kernel/network/386/ether8390.c)7]] */

typedef struct Hdr Hdr;


/*s: struct [[Hdr]]([[(kernel/network/386/ether8390.c)]]) */
struct Hdr {
    uchar	status;
    uchar	next;
    uchar	len0;
    uchar	len1;
};
/*e: struct [[Hdr]]([[(kernel/network/386/ether8390.c)]]) */


/*s: function [[dp8390setea]] */
void
dp8390setea(Ether* ether)
{
    int i;
    uchar cr;
    Dp8390 *ctlr;

    ctlr = ether->ctlr;

    /*
     * Set the ethernet address into the chip.
     * Take care to restore the command register
     * afterwards. Don't care about multicast
     * addresses as multicast is never enabled
     * (currently).
     */
    ilock(ctlr);
    cr = regr(ctlr, Cr) & ~Txp;
    regw(ctlr, Cr, Page1|(~(Ps1|Ps0) & cr));
    for(i = 0; i < Eaddrlen; i++)
        regw(ctlr, Par0+i, ether->ea[i]);
    regw(ctlr, Cr, cr);
    iunlock(ctlr);
}
/*e: function [[dp8390setea]] */

/*s: function [[_dp8390read]] */
static void*
_dp8390read(Dp8390* ctlr, void* to, ulong from, ulong len)
{
    uchar cr;
    int timo;

    /*
     * Read some data at offset 'from' in the card's memory
     * using the DP8390 remote DMA facility, and place it at
     * 'to' in main memory, via the I/O data port.
     */
    cr = regr(ctlr, Cr) & ~Txp;
    regw(ctlr, Cr, Page0|RdABORT|Sta);
    regw(ctlr, Isr, Rdc);

    /*
     * Set up the remote DMA address and count.
     */
    len = ROUNDUP(len, ctlr->width);
    regw(ctlr, Rbcr0, len & 0xFF);
    regw(ctlr, Rbcr1, (len>>8) & 0xFF);
    regw(ctlr, Rsar0, from & 0xFF);
    regw(ctlr, Rsar1, (from>>8) & 0xFF);

    /*
     * Start the remote DMA read and suck the data
     * out of the I/O port.
     */
    regw(ctlr, Cr, Page0|RdREAD|Sta);
    rdread(ctlr, to, len);

    /*
     * Wait for the remote DMA to complete. The timeout
     * is necessary because this routine may be called on
     * a non-existent chip during initialisation and, due
     * to the miracles of the bus, it's possible to get this
     * far and still be talking to a slot full of nothing.
     */
    for(timo = 10000; (regr(ctlr, Isr) & Rdc) == 0 && timo; timo--)
            ;

    regw(ctlr, Isr, Rdc);
    regw(ctlr, Cr, cr);

    return to;
}
/*e: function [[_dp8390read]] */

/*s: function [[dp8390read]] */
void*
dp8390read(Dp8390* ctlr, void* to, ulong from, ulong len)
{
    void *v;

    ilock(ctlr);
    v = _dp8390read(ctlr, to, from, len);
    iunlock(ctlr);

    return v;
}
/*e: function [[dp8390read]] */

/*s: function [[dp8390write]] */
static void*
dp8390write(Dp8390* ctlr, ulong to, void* from, ulong len)
{
    ulong crda;
    uchar cr;
    int timo, width;

top:
    /*
     * Write some data to offset 'to' in the card's memory
     * using the DP8390 remote DMA facility, reading it at
     * 'from' in main memory, via the I/O data port.
     */
    cr = regr(ctlr, Cr) & ~Txp;
    regw(ctlr, Cr, Page0|RdABORT|Sta);
    regw(ctlr, Isr, Rdc);

    len = ROUNDUP(len, ctlr->width);

    /*
     * Set up the remote DMA address and count.
     * This is straight from the DP8390[12D] datasheet,
     * hence the initial set up for read.
     * Assumption here that the A7000 EtherV card will
     * never need a dummyrr.
     */
    if(ctlr->dummyrr && (ctlr->width == 1 || ctlr->width == 2)){
        if(ctlr->width == 2)
            width = 1;
        else
            width = 0;
        crda = to-1-width;
        regw(ctlr, Rbcr0, (len+1+width) & 0xFF);
        regw(ctlr, Rbcr1, ((len+1+width)>>8) & 0xFF);
        regw(ctlr, Rsar0, crda & 0xFF);
        regw(ctlr, Rsar1, (crda>>8) & 0xFF);
        regw(ctlr, Cr, Page0|RdREAD|Sta);
    
        for(timo=0;; timo++){
            if(timo > 10000){
                print("ether8390: dummyrr timeout; assuming nodummyrr\n");
                ctlr->dummyrr = 0;
                goto top;
            }
            crda = regr(ctlr, Crda0);
            crda |= regr(ctlr, Crda1)<<8;
            if(crda == to){
                /*
                 * Start the remote DMA write and make sure
                 * the registers are correct.
                 */
                regw(ctlr, Cr, Page0|RdWRITE|Sta);
    
                crda = regr(ctlr, Crda0);
                crda |= regr(ctlr, Crda1)<<8;
                if(crda != to)
                    panic("crda write %lud to %lud\n", crda, to);
    
                break;
            }
        }
    }
    else{
        regw(ctlr, Rsar0, to & 0xFF);
        regw(ctlr, Rsar1, (to>>8) & 0xFF);
        regw(ctlr, Rbcr0, len & 0xFF);
        regw(ctlr, Rbcr1, (len>>8) & 0xFF);
        regw(ctlr, Cr, Page0|RdWRITE|Sta);
    }

    /*
     * Pump the data into the I/O port
     * then wait for the remote DMA to finish.
     */
    rdwrite(ctlr, from, len);
    for(timo = 10000; (regr(ctlr, Isr) & Rdc) == 0 && timo; timo--)
            ;

    regw(ctlr, Isr, Rdc);
    regw(ctlr, Cr, cr);

    return (void*)to;
}
/*e: function [[dp8390write]] */

/*s: function [[ringinit]] */
static void
ringinit(Dp8390* ctlr)
{
    regw(ctlr, Pstart, ctlr->pstart);
    regw(ctlr, Pstop, ctlr->pstop);
    regw(ctlr, Bnry, ctlr->pstop-1);

    regw(ctlr, Cr, Page1|RdABORT|Stp);
    regw(ctlr, Curr, ctlr->pstart);
    regw(ctlr, Cr, Page0|RdABORT|Stp);

    ctlr->nxtpkt = ctlr->pstart;
}
/*e: function [[ringinit]] */

/*s: function [[getcurr]] */
static uchar
getcurr(Dp8390* ctlr)
{
    uchar cr, curr;

    cr = regr(ctlr, Cr) & ~Txp;
    regw(ctlr, Cr, Page1|(~(Ps1|Ps0) & cr));
    curr = regr(ctlr, Curr);
    regw(ctlr, Cr, cr);

    return curr;
}
/*e: function [[getcurr]] */

/*s: function [[receive]]([[(kernel/network/386/ether8390.c)]]) */
static void
receive(Ether* ether)
{
    Dp8390 *ctlr;
    uchar curr, *p;
    Hdr hdr;
    ulong count, data, len;
    Block *bp;

    ctlr = ether->ctlr;
    for(curr = getcurr(ctlr); ctlr->nxtpkt != curr; curr = getcurr(ctlr)){
        data = ctlr->nxtpkt*Dp8390BufSz;
        if(ctlr->ram)
            memmove(&hdr, (void*)(ether->mem+data), sizeof(Hdr));
        else
            _dp8390read(ctlr, &hdr, data, sizeof(Hdr));

        /*
         * Don't believe the upper byte count, work it
         * out from the software next-page pointer and
         * the current next-page pointer.
         */
        if(hdr.next > ctlr->nxtpkt)
            len = hdr.next - ctlr->nxtpkt - 1;
        else
            len = (ctlr->pstop-ctlr->nxtpkt) + (hdr.next-ctlr->pstart) - 1;
        if(hdr.len0 > (Dp8390BufSz-sizeof(Hdr)))
            len--;

        len = ((len<<8)|hdr.len0)-4;

        /*
         * Chip is badly scrogged, reinitialise the ring.
         */
        if(hdr.next < ctlr->pstart || hdr.next >= ctlr->pstop
          || len < 60 || len > sizeof(Etherpkt)){
            print("dp8390: H%2.2ux+%2.2ux+%2.2ux+%2.2ux,%lud\n",
                hdr.status, hdr.next, hdr.len0, hdr.len1, len);
            regw(ctlr, Cr, Page0|RdABORT|Stp);
            ringinit(ctlr);
            regw(ctlr, Cr, Page0|RdABORT|Sta);

            return;
        }

        /*
         * If it's a good packet read it in to the software buffer.
         * If the packet wraps round the hardware ring, read it in
         * two pieces.
         */
        if((hdr.status & (Fo|Fae|Crce|Prxok)) == Prxok && (bp = iallocb(len))){
            p = bp->rp;
            bp->wp = p+len;
            data += sizeof(Hdr);

            if((data+len) >= ctlr->pstop*Dp8390BufSz){
                count = ctlr->pstop*Dp8390BufSz - data;
                if(ctlr->ram)
                    memmove(p, (void*)(ether->mem+data), count);
                else
                    _dp8390read(ctlr, p, data, count);
                p += count;
                data = ctlr->pstart*Dp8390BufSz;
                len -= count;
            }
            if(len){
                if(ctlr->ram)
                    memmove(p, (void*)(ether->mem+data), len);
                else
                    _dp8390read(ctlr, p, data, len);
            }

            /*
             * Copy the packet to whoever wants it.
             */
            etheriq(ether, bp, 1);
        }

        /*
         * Finished with this packet, update the
         * hardware and software ring pointers.
         */
        ctlr->nxtpkt = hdr.next;

        hdr.next--;
        if(hdr.next < ctlr->pstart)
            hdr.next = ctlr->pstop-1;
        regw(ctlr, Bnry, hdr.next);
    }
}
/*e: function [[receive]]([[(kernel/network/386/ether8390.c)]]) */

/*s: function [[txstart]] */
static void
txstart(Ether* ether)
{
    int len;
    Dp8390 *ctlr;
    Block *bp;
    uchar minpkt[ETHERMINTU], *rp;

    ctlr = ether->ctlr;

    /*
     * This routine is called both from the top level and from interrupt
     * level and expects to be called with ctlr already locked.
     */
    if(ctlr->txbusy)
        return;
    bp = qget(ether->oq);
    if(bp == nil)
        return;

    /*
     * Make sure the packet is of minimum length;
     * copy it to the card's memory by the appropriate means;
     * start the transmission.
     */
    len = BLEN(bp);
    rp = bp->rp;
    if(len < ETHERMINTU){
        rp = minpkt;
        memmove(rp, bp->rp, len);
        memset(rp+len, 0, ETHERMINTU-len);
        len = ETHERMINTU;
    }

    if(ctlr->ram)
        memmove((void*)(ether->mem+ctlr->tstart*Dp8390BufSz), rp, len);
    else
        dp8390write(ctlr, ctlr->tstart*Dp8390BufSz, rp, len);
    freeb(bp);

    regw(ctlr, Tbcr0, len & 0xFF);
    regw(ctlr, Tbcr1, (len>>8) & 0xFF);
    regw(ctlr, Cr, Page0|RdABORT|Txp|Sta);

    ether->outpackets++;
    ctlr->txbusy = 1;
}
/*e: function [[txstart]] */

/*s: function [[transmit]]([[(kernel/network/386/ether8390.c)]]) */
static void
transmit(Ether* ether)
{
    Dp8390 *ctlr;

    ctlr = ether->ctlr;

    ilock(ctlr);
    txstart(ether);
    iunlock(ctlr);
}
/*e: function [[transmit]]([[(kernel/network/386/ether8390.c)]]) */

/*s: function [[overflow]] */
static void
overflow(Ether *ether)
{
    Dp8390 *ctlr;
    uchar txp;
    int resend;

    ctlr = ether->ctlr;

    /*
     * The following procedure is taken from the DP8390[12D] datasheet,
     * it seems pretty adamant that this is what has to be done.
     */
    txp = regr(ctlr, Cr) & Txp;
    regw(ctlr, Cr, Page0|RdABORT|Stp);
    arch_delay(2);
    regw(ctlr, Rbcr0, 0);
    regw(ctlr, Rbcr1, 0);

    resend = 0;
    if(txp && (regr(ctlr, Isr) & (Txe|Ptx)) == 0)
        resend = 1;

    regw(ctlr, Tcr, LpbkNIC);
    regw(ctlr, Cr, Page0|RdABORT|Sta);
    receive(ether);
    regw(ctlr, Isr, Ovw);
    regw(ctlr, Tcr, LpbkNORMAL);

    if(resend)
        regw(ctlr, Cr, Page0|RdABORT|Txp|Sta);
}
/*e: function [[overflow]] */

/*s: function [[interrupt]]([[(kernel/network/386/ether8390.c)]]) */
static void
interrupt(Ureg*, void* arg)
{
    Ether *ether;
    Dp8390 *ctlr;
    uchar isr, r;

    ether = arg;
    ctlr = ether->ctlr;

    /*
     * While there is something of interest,
     * clear all the interrupts and process.
     */
    ilock(ctlr);
    regw(ctlr, Imr, 0x00);
    while(isr = (regr(ctlr, Isr) & (Cnt|Ovw|Txe|Rxe|Ptx|Prx))){
        if(isr & Ovw){
            overflow(ether);
            regw(ctlr, Isr, Ovw);
            ether->overflows++;
        }

        /*
         * Packets have been received.
         * Take a spin round the ring.
         */
        if(isr & (Rxe|Prx)){
            receive(ether);
            regw(ctlr, Isr, Rxe|Prx);
        }

        /*
         * A packet completed transmission, successfully or
         * not. Start transmission on the next buffered packet,
         * and wake the output routine.
         */
        if(isr & (Txe|Ptx)){
            r = regr(ctlr, Tsr);
            if((isr & Txe) && (r & (Cdh|Fu|Crs|Abt))){
                print("dp8390: Tsr %#2.2ux", r);
                ether->oerrs++;
            }

            regw(ctlr, Isr, Txe|Ptx);

            if(isr & Ptx)
                ether->outpackets++;
            ctlr->txbusy = 0;
            txstart(ether);
        }

        if(isr & Cnt){
            ether->frames += regr(ctlr, Ref0);
            ether->crcs += regr(ctlr, Ref1);
            ether->buffs += regr(ctlr, Ref2);
            regw(ctlr, Isr, Cnt);
        }
    }
    regw(ctlr, Imr, Cnt|Ovw|Txe|Rxe|Ptx|Prx);
    iunlock(ctlr);
}
/*e: function [[interrupt]]([[(kernel/network/386/ether8390.c)]]) */

/*s: global [[allmar]] */
static uchar allmar[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
/*e: global [[allmar]] */

/*s: function [[setfilter]] */
static void
setfilter(Ether *ether, Dp8390 *ctlr)
{
    uchar r, cr;
    int i;
    uchar *mar;

    r = Ab;
    mar = 0;
    if(ether->prom){
        r |= Pro|Am;
        mar = allmar;
    } else if(ether->nmaddr){
        r |= Am;
        mar = ctlr->mar;
    }
    if(mar){
        cr = regr(ctlr, Cr) & ~Txp;
        regw(ctlr, Cr, Page1|(~(Ps1|Ps0) & cr));
        for(i = 0; i < 8; i++)
            regw(ctlr, Mar0+i, *(mar++));
        regw(ctlr, Cr, cr);
    }
    regw(ctlr, Rcr, r);
}
/*e: function [[setfilter]] */

/*s: function [[promiscuous]] */
static void
promiscuous(void *arg, int )
{
    Ether *ether;
    Dp8390 *ctlr;

    ether = arg;
    ctlr = ether->ctlr;

    ilock(ctlr);
    setfilter(ether, ctlr);
    iunlock(ctlr);
}
/*e: function [[promiscuous]] */

/*s: function [[setbit]] */
static void
setbit(Dp8390 *ctlr, int bit, int on)
{
    int i, h;

    i = bit/8;
    h = bit%8;
    if(on){
        if(++(ctlr->mref[bit]) == 1)
            ctlr->mar[i] |= 1<<h;
    } else {
        if(--(ctlr->mref[bit]) <= 0){
            ctlr->mref[bit] = 0;
            ctlr->mar[i] &= ~(1<<h);
        }
    }
}
/*e: function [[setbit]] */

/*s: global [[reverse]] */
static uchar reverse[64];
/*e: global [[reverse]] */

/*s: function [[multicast]] */
static void
multicast(void* arg, uchar *addr, int on)
{
    Ether *ether;
    Dp8390 *ctlr;
    int i;
    ulong h;

    ether = arg;
    ctlr = ether->ctlr;
    if(reverse[1] == 0){
        for(i = 0; i < 64; i++)
            reverse[i] = ((i&1)<<5) | ((i&2)<<3) | ((i&4)<<1)
                   | ((i&8)>>1) | ((i&16)>>3) | ((i&32)>>5);
    }

    /*
     *  change filter bits
     */
    h = ethercrc(addr, 6);
    ilock(ctlr);
    setbit(ctlr, reverse[h&0x3f], on);
    setfilter(ether, ctlr);
    iunlock(ctlr);
}
/*e: function [[multicast]] */

/*s: function [[attach]] */
static void
attach(Ether* ether)
{
    Dp8390 *ctlr;
    uchar r;

    ctlr = ether->ctlr;

    /*
     * Enable the chip for transmit/receive.
     * The init routine leaves the chip in monitor
     * mode. Clear the missed-packet counter, it
     * increments while in monitor mode.
     * Sometimes there's an interrupt pending at this
     * point but there's nothing in the Isr, so
     * any pending interrupts are cleared and the
     * mask of acceptable interrupts is enabled here.
     */
    r = Ab;
    if(ether->prom)
        r |= Pro;
    if(ether->nmaddr)
        r |= Am;
    ilock(ctlr);
    regw(ctlr, Isr, 0xFF);
    regw(ctlr, Imr, Cnt|Ovw|Txe|Rxe|Ptx|Prx);
    regw(ctlr, Rcr, r);
    r = regr(ctlr, Ref2);
    regw(ctlr, Tcr, LpbkNORMAL);
    iunlock(ctlr);
    USED(r);
}
/*e: function [[attach]] */

/*s: function [[disable]] */
static void
disable(Dp8390* ctlr)
{
    int timo;

    /*
     * Stop the chip. Set the Stp bit and wait for the chip
     * to finish whatever was on its tiny mind before it sets
     * the Rst bit.
     * The timeout is needed because there may not be a real
     * chip there if this is called when probing for a device
     * at boot.
     */
    regw(ctlr, Cr, Page0|RdABORT|Stp);
    regw(ctlr, Rbcr0, 0);
    regw(ctlr, Rbcr1, 0);
    for(timo = 10000; (regr(ctlr, Isr) & Rst) == 0 && timo; timo--)
            ;
}
/*e: function [[disable]] */

/*s: function [[shutdown]] */
static void
shutdown(Ether *ether)
{
    Dp8390 *ctlr;

    ctlr = ether->ctlr;
    disable(ctlr);
}
/*e: function [[shutdown]] */

/*s: function [[dp8390reset]] */
int
dp8390reset(Ether* ether)
{
    Dp8390 *ctlr;

    ctlr = ether->ctlr;

    /*
     * This is the initialisation procedure described
     * as 'mandatory' in the datasheet, with references
     * to the 3C503 technical reference manual.
     */ 
    disable(ctlr);
    if(ctlr->width != 1)
        regw(ctlr, Dcr, Ft4WORD|Ls|Wts);
    else
        regw(ctlr, Dcr, Ft4WORD|Ls);

    regw(ctlr, Rbcr0, 0);
    regw(ctlr, Rbcr1, 0);

    regw(ctlr, Tcr, LpbkNIC);
    regw(ctlr, Rcr, Mon);

    /*
     * Init the ring hardware and software ring pointers.
     * Can't initialise ethernet address as it may not be
     * known yet.
     */
    ringinit(ctlr);
    regw(ctlr, Tpsr, ctlr->tstart);

    /*
     * Clear any pending interrupts and mask then all off.
     */
    regw(ctlr, Isr, 0xFF);
    regw(ctlr, Imr, 0);

    /*
     * Leave the chip initialised,
     * but in monitor mode.
     */
    regw(ctlr, Cr, Page0|RdABORT|Sta);



    /*
     * Set up the software configuration.
     */
    ether->attach = attach;
    ether->shutdown = shutdown;

    ether->transmit = transmit;
    ether->interrupt = interrupt;
    ether->ifstat = 0;

    ether->promiscuous = promiscuous;
    ether->multicast = multicast;
    ether->arg = ether;

    return 0;
}
/*e: function [[dp8390reset]] */
/*e: kernel/network/386/ether8390.c */
