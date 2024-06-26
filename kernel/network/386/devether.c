/*s: kernel/network/386/devether.c */
#include "u.h"
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

#include "io.h"
#include "pool.h"
#include <ureg.h>
#include "../port/netif.h"
#include "../port/etherif.h"

/*s: global etherxx (kernel) */
static Ether *etherxx[MaxEther];
/*e: global etherxx (kernel) */

/*s: function [[etherattach]] */
Chan*
etherattach(char* spec)
{
    ulong ctlrno;
    char *p;
    Chan *chan;

    ctlrno = 0;
    if(spec && *spec){
        ctlrno = strtoul(spec, &p, 0);
        if((ctlrno == 0 && p == spec) || *p || (ctlrno >= MaxEther))
            error(Ebadarg);
    }
    if(etherxx[ctlrno] == nil)
        error(Enodev);

    chan = devattach('l', spec);
    if(waserror()){
        chanfree(chan);
        nexterror();
    }
    chan->dev = ctlrno;
    if(etherxx[ctlrno]->attach)
        // Ethernet controller dispatch
        etherxx[ctlrno]->attach(etherxx[ctlrno]);

    poperror();
    return chan;
}
/*e: function [[etherattach]] */

/*s: function [[etherwalk]] */
static Walkqid*
etherwalk(Chan* chan, Chan* nchan, char** name, int nname)
{
    return netifwalk(etherxx[chan->dev], chan, nchan, name, nname);
}
/*e: function [[etherwalk]] */

/*s: function [[etherstat]] */
static int
etherstat(Chan* chan, uchar* dp, int n)
{
    return netifstat(etherxx[chan->dev], chan, dp, n);
}
/*e: function [[etherstat]] */

/*s: function [[etheropen]] */
static Chan*
etheropen(Chan* chan, int omode)
{
    return netifopen(etherxx[chan->dev], chan, omode);
}
/*e: function [[etheropen]] */

/*s: function [[ethercreate]] */
static void
ethercreate(Chan*, char*, int, ulong)
{
}
/*e: function [[ethercreate]] */

/*s: function [[etherclose]] */
static void
etherclose(Chan* chan)
{
    netifclose(etherxx[chan->dev], chan);
}
/*e: function [[etherclose]] */

/*s: function [[etherread]] */
static long
etherread(Chan* chan, void* buf, long n, vlong off)
{
    Ether *ether;
    ulong offset = off;

    ether = etherxx[chan->dev];
    if((chan->qid.type & QTDIR) == 0 && ether->ifstat){
        /*
         * With some controllers it is necessary to reach
         * into the chip to extract statistics.
         */
        if(NETTYPE(chan->qid.path) == Nifstatqid)
            return ether->ifstat(ether, buf, n, offset);
        else if(NETTYPE(chan->qid.path) == Nstatqid)
            ether->ifstat(ether, buf, 0, offset);
    }

    return netifread(ether, chan, buf, n, offset);
}
/*e: function [[etherread]] */

/*s: function [[etherbread]] */
static Block*
etherbread(Chan* chan, long n, ulong offset)
{
    return netifbread(etherxx[chan->dev], chan, n, offset);
}
/*e: function [[etherbread]] */

/*s: function [[etherwstat]] */
static int
etherwstat(Chan* chan, uchar* dp, int n)
{
    return netifwstat(etherxx[chan->dev], chan, dp, n);
}
/*e: function [[etherwstat]] */

/*s: function [[etherrtrace]] */
static void
etherrtrace(Netfile* f, Etherpkt* pkt, int len)
{
    int i, n;
    Block *bp;

    if(qwindow(f->in) <= 0)
        return;
    if(len > 58)
        n = 58;
    else
        n = len;
    bp = iallocb(64);
    if(bp == nil)
        return;
    memmove(bp->wp, pkt->d, n);
    i = TK2MS(CPUS(0)->ticks);
    bp->wp[58] = len>>8;
    bp->wp[59] = len;
    bp->wp[60] = i>>24;
    bp->wp[61] = i>>16;
    bp->wp[62] = i>>8;
    bp->wp[63] = i;
    bp->wp += 64;
    qpass(f->in, bp);
}
/*e: function [[etherrtrace]] */

/*s: function [[etheriq]] */
Block*
etheriq(Ether* ether, Block* bp, int fromwire)
{
    Etherpkt *pkt;
    ushort type;
    int len, multi, tome, fromme;
    Netfile **ep, *f, **fp, *fx;
    Block *xbp;

    ether->inpackets++;

    pkt = (Etherpkt*)bp->rp;
    len = BLEN(bp);
    type = (pkt->type[0]<<8)|pkt->type[1];
    fx = 0;
    ep = &ether->f[Ntypes];

    multi = pkt->d[0] & 1;
    /* check for valid multicast addresses */
    if(multi && memcmp(pkt->d, ether->bcast, sizeof(pkt->d)) != 0 && ether->prom == 0){
        if(!activemulti(ether, pkt->d, sizeof(pkt->d))){
            if(fromwire){
                freeb(bp);
                bp = 0;
            }
            return bp;
        }
    }

    /* is it for me? */
    tome = memcmp(pkt->d, ether->ea, sizeof(pkt->d)) == 0;
    fromme = memcmp(pkt->s, ether->ea, sizeof(pkt->s)) == 0;

    /*
     * Multiplex the packet to all the connections which want it.
     * If the packet is not to be used subsequently (fromwire != 0),
     * attempt to simply pass it into one of the connections, thereby
     * saving a copy of the data (usual case hopefully).
     */
    for(fp = ether->f; fp < ep; fp++){
        if(f = *fp)
        if(f->type == type || f->type < 0)
        if(tome || multi || f->prom){
            /* Don't want to hear bridged packets */
            if(f->bridge && !fromwire && !fromme)
                continue;
            if(!f->headersonly){
                if(fromwire && fx == 0)
                    fx = f;
                else if(xbp = iallocb(len)){
                    memmove(xbp->wp, pkt, len);
                    xbp->wp += len;
                    if(qpass(f->in, xbp) < 0){
                        // print("soverflow for f->in\n");
                        ether->soverflows++;
                    }
                }
                else{
                    // print("soverflow iallocb\n");
                    ether->soverflows++;
                }
            }
            else
                etherrtrace(f, pkt, len);
        }
    }

    if(fx){
        if(qpass(fx->in, bp) < 0){
            // print("soverflow for fx->in\n");
            ether->soverflows++;
        }
        return 0;
    }
    if(fromwire){
        freeb(bp);
        return 0;
    }

    return bp;
}
/*e: function [[etheriq]] */

/*s: function [[etheroq]] */
static int
etheroq(Ether* ether, Block* bp)
{
    int len, loopback, s;
    Etherpkt *pkt;

    ether->outpackets++;

    /*
     * Check if the packet has to be placed back onto the input queue,
     * i.e. if it's a loopback or broadcast packet or the interface is
     * in promiscuous mode.
     * If it's a loopback packet indicate to etheriq that the data isn't
     * needed and return, etheriq will pass-on or free the block.
     * To enable bridging to work, only packets that were originated
     * by this interface are fed back.
     */
    pkt = (Etherpkt*)bp->rp;
    len = BLEN(bp);
    loopback = memcmp(pkt->d, ether->ea, sizeof(pkt->d)) == 0;
    if(loopback || memcmp(pkt->d, ether->bcast, sizeof(pkt->d)) == 0 || ether->prom){
        s = arch_splhi();
        etheriq(ether, bp, 0);
        arch_splx(s);
    }

    if(!loopback){
        if(qfull(ether->oq))
            print("etheroq: WARNING: ether->oq full!\n");
        qbwrite(ether->oq, bp);
        if(ether->transmit != nil)
            ether->transmit(ether);
    } else
        freeb(bp);

    return len;
}
/*e: function [[etheroq]] */

/*s: function [[etherwrite]] */
static long
etherwrite(Chan* chan, void* buf, long n, vlong)
{
    Ether *ether;
    Block *bp;
    int nn, onoff;
    Cmdbuf *cb;

    ether = etherxx[chan->dev];
    if(NETTYPE(chan->qid.path) != Ndataqid) {
        nn = netifwrite(ether, chan, buf, n);
        if(nn >= 0)
            return nn;
        cb = parsecmd(buf, n);
        if(cb->f[0] && strcmp(cb->f[0], "nonblocking") == 0){
            if(cb->nf <= 1)
                onoff = 1;
            else
                onoff = atoi(cb->f[1]);
            qnoblock(ether->oq, onoff);
            free(cb);
            return n;
        }
        free(cb);
        if(ether->ctl != nil)
            return ether->ctl(ether, buf, n);

        error(Ebadctl);
    }

    if(n > ether->mtu)
        error(Etoobig);
    if(n < ether->minmtu)
        error(Etoosmall);

    bp = allocb(n);
    if(waserror()){
        freeb(bp);
        nexterror();
    }
    memmove(bp->rp, buf, n);
    memmove(bp->rp+Eaddrlen, ether->ea, Eaddrlen);
    poperror();
    bp->wp += n;

    return etheroq(ether, bp);
}
/*e: function [[etherwrite]] */

/*s: function [[etherbwrite]] */
static long
etherbwrite(Chan* chan, Block* bp, ulong)
{
    Ether *ether;
    long n;

    n = BLEN(bp);
    if(NETTYPE(chan->qid.path) != Ndataqid){
        if(waserror()) {
            freeb(bp);
            nexterror();
        }
        n = etherwrite(chan, bp->rp, n, 0);
        poperror();
        freeb(bp);
        return n;
    }
    ether = etherxx[chan->dev];

    if(n > ether->mtu){
        freeb(bp);
        error(Etoobig);
    }
    if(n < ether->minmtu){
        freeb(bp);
        error(Etoosmall);
    }

    return etheroq(ether, bp);
}
/*e: function [[etherbwrite]] */

/*s: global [[cards]] */
static struct {
    char*   type;
    int (*reset)(Ether*);
} cards[MaxEther+1];
/*e: global [[cards]] */

/*s: function [[addethercard]] */
void
addethercard(char* t, int (*r)(Ether*))
{
    static int ncard;

    if(ncard == MaxEther)
        panic("too many ether cards");
    cards[ncard].type = t;
    cards[ncard].reset = r;
    ncard++;
}
/*e: function [[addethercard]] */

/*s: function [[parseether]]([[(kernel/network/386/devether.c)]]) */
int
parseether(uchar *to, char *from)
{
    char nip[4];
    char *p;
    int i;

    p = from;
    for(i = 0; i < Eaddrlen; i++){
        if(*p == 0)
            return -1;
        nip[0] = *p++;
        if(*p == 0)
            return -1;
        nip[1] = *p++;
        nip[2] = 0;
        to[i] = strtoul(nip, 0, 16);
        if(*p == ':')
            p++;
    }
    return 0;
}
/*e: function [[parseether]]([[(kernel/network/386/devether.c)]]) */

/*s: function [[etherprobe]] */
static Ether*
etherprobe(int cardno, int ctlrno)
{
    int i, lg;
    ulong mb, bsz;
    Ether *ether;
    char buf[128], name[32];

    ether = malloc(sizeof(Ether));
    if(ether == nil)
        error(Enomem);
    memset(ether, 0, sizeof(Ether));
    ether->ctlrno = ctlrno;
    ether->tbdf = BUSUNKNOWN;
    ether->mbps = 10;
    ether->minmtu = ETHERMINTU;
    ether->maxmtu = ETHERMAXTU;
    ether->mtu = ETHERMAXTU;

    if(cardno < 0){
        if(arch_isaconfig("ether", ctlrno, ether) == 0){
            free(ether);
            return nil;
        }
        for(cardno = 0; cards[cardno].type; cardno++){
            if(cistrcmp(cards[cardno].type, ether->type))
                continue;
            for(i = 0; i < ether->nopt; i++){
                if(strncmp(ether->opt[i], "ea=", 3))
                    continue;
                if(parseether(ether->ea, &ether->opt[i][3]))
                    memset(ether->ea, 0, Eaddrlen);
            }
            break;
        }
    }

    if(cardno >= MaxEther || cards[cardno].type == nil){
        free(ether);
        return nil;
    }
    if(cards[cardno].reset(ether) < 0){
        free(ether);
        return nil;
    }

    /*
     * IRQ2 doesn't really exist, it's used to gang the interrupt
     * controllers together. A device set to IRQ2 will appear on
     * the second interrupt controller as IRQ9.
     */
    if(ether->irq == 2)
        ether->irq = 9;
    snprint(name, sizeof(name), "ether%d", ctlrno);

    /*
     * If ether->irq is <0, it is a hack to indicate no interrupt
     * used by ethersink.
     */
    if(ether->irq >= 0)
        arch_intrenable(ether->irq, ether->interrupt, ether, ether->tbdf, name);

    i = sprint(buf, "#l%d: %s: ", ctlrno, cards[cardno].type);
    if(ether->mbps >= 1000)
        i += sprint(buf+i, "%dGbps", ether->mbps/1000);
    else
        i += sprint(buf+i, "%dMbps", ether->mbps);
    i += sprint(buf+i, " port 0x%luX irq %d", ether->port, ether->irq);
    if(ether->mem)
        i += sprint(buf+i, " addr 0x%luX", ether->mem);
    if(ether->size)
        i += sprint(buf+i, " size 0x%luX", ether->size);
    i += sprint(buf+i, ": %2.2ux%2.2ux%2.2ux%2.2ux%2.2ux%2.2ux",
        ether->ea[0], ether->ea[1], ether->ea[2],
        ether->ea[3], ether->ea[4], ether->ea[5]);
    sprint(buf+i, "\n");
    print(buf);

    /*
     * input queues are allocated by ../port/netif.c:/^openfile.
     * the size will be the last argument to netifinit() below.
     *
     * output queues should be small, to minimise `bufferbloat',
     * which confuses tcp's feedback loop.  at 1Gb/s, it only takes
     * ~15µs to transmit a full-sized non-jumbo packet.
     */

    /* compute log10(ether->mbps) into lg */
    for(lg = 0, mb = ether->mbps; mb >= 10; lg++)
        mb /= 10;
    if (lg > 14)            /* sanity cap; 2**(14+15) = 2²⁹ */
        lg = 14;

    /* allocate larger input queues for higher-speed interfaces */
    bsz = 1UL << (lg + 15);     /* 2ⁱ⁵ = 32K, bsz = 2ⁿ × 32K */
    while (bsz > mainmem->maxsize / 8 && bsz > 128*1024)    /* sanity */
        bsz /= 2;
    netifinit(ether, name, Ntypes, bsz);

    if(ether->oq == nil)
        ether->oq = qopen(1 << (lg + 13), Qmsg, 0, 0);
    if(ether->oq == nil)
        panic("etherreset %s: can't allocate output queue", name);

    ether->alen = Eaddrlen;
    memmove(ether->addr, ether->ea, Eaddrlen);
    memset(ether->bcast, 0xFF, Eaddrlen);

    return ether;
}
/*e: function [[etherprobe]] */

/*s: function [[etherreset]] */
static void
etherreset(void)
{
    Ether *ether;
    int cardno, ctlrno;

    for(ctlrno = 0; ctlrno < MaxEther; ctlrno++){
        if((ether = etherprobe(-1, ctlrno)) == nil)
            continue;
        etherxx[ctlrno] = ether;
    }

    if(getconf("*noetherprobe"))
        return;

    cardno = ctlrno = 0;
    while(cards[cardno].type != nil && ctlrno < MaxEther){
        if(etherxx[ctlrno] != nil){
            ctlrno++;
            continue;
        }
        if((ether = etherprobe(cardno, ctlrno)) == nil){
            cardno++;
            continue;
        }
        etherxx[ctlrno] = ether;
        ctlrno++;
    }
}
/*e: function [[etherreset]] */

/*s: function [[ethershutdown]] */
static void
ethershutdown(void)
{
    Ether *ether;
    int i;

    for(i = 0; i < MaxEther; i++){
        ether = etherxx[i];
        if(ether == nil)
            continue;
        if(ether->shutdown == nil) {
            print("#l%d: no shutdown function\n", i);
            continue;
        }
        (*ether->shutdown)(ether);
    }
}
/*e: function [[ethershutdown]] */

/*s: global [[etherdevtab]] */
Dev etherdevtab = {
    .dc       =    'l',
    .name     =    "ether",

    .attach   =    etherattach,
    .walk     =    etherwalk,
    .open     =    etheropen,
    .close    =    etherclose,
    .read     =    etherread,
    .write    =    etherwrite,
    .stat     =    etherstat,
    .wstat    =    etherwstat,
               
    .reset    =    etherreset,
    .init     =    devinit,
    .shutdown =    ethershutdown,
    .create   =    ethercreate,
    .bread    =    etherbread,
    .bwrite   =    etherbwrite,
    .remove   =    devremove,
};
/*e: global [[etherdevtab]] */

/*s: constant [[POLY]] */
#define POLY 0xedb88320
/*e: constant [[POLY]] */

/*s: function [[ethercrc]] */
/* really slow 32 bit crc for ethers */
ulong
ethercrc(uchar *p, int len)
{
  int i, j;
  ulong crc, b;

  crc = 0xffffffff;
  for(i = 0; i < len; i++){
      b = *p++;
      for(j = 0; j < 8; j++){
          crc = (crc>>1) ^ (((crc^b) & 1) ? POLY : 0);
          b >>= 1;
      }
  }
  return crc;
}
/*e: function [[ethercrc]] */
/*e: kernel/network/386/devether.c */
