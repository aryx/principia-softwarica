/*s: kernel/network/ip/ip.c */
#include    "u.h"
#include    "../port/lib.h"
#include    "mem.h"
#include    "dat.h"
#include    "fns.h"
#include    "../port/error.h"

#include    "ip.h"

/*s: macro BLKIPVER */
#define BLKIPVER(xp)    (((Ip4hdr*)((xp)->rp))->vihl&0xF0)
/*e: macro BLKIPVER */

/*s: global statnames */
static char *statnames[] =
{
[Forwarding]    "Forwarding",
[DefaultTTL]    "DefaultTTL",
[InReceives]    "InReceives",
[InHdrErrors]   "InHdrErrors",
[InAddrErrors]  "InAddrErrors",
[ForwDatagrams] "ForwDatagrams",
[InUnknownProtos]   "InUnknownProtos",

[InDiscards]    "InDiscards",
[InDelivers]    "InDelivers",

[OutRequests]   "OutRequests",
[OutDiscards]   "OutDiscards",
[OutNoRoutes]   "OutNoRoutes",

[ReasmTimeout]  "ReasmTimeout",
[ReasmReqds]    "ReasmReqds",
[ReasmOKs]  "ReasmOKs",
[ReasmFails]    "ReasmFails",

[FragOKs]   "FragOKs",
[FragFails] "FragFails",
[FragCreates]   "FragCreates",
};
/*e: global statnames */

/*s: macro BLKIP */
#define BLKIP(xp)   ((Ip4hdr*)((xp)->rp))
/*e: macro BLKIP */
/*s: macro BKFG */
/*
 * This sleazy macro relies on the media header size being
 * larger than sizeof(Ipfrag). ipreassemble checks this is true
 */
#define BKFG(xp)    ((Ipfrag*)((xp)->base))
/*e: macro BKFG */

ushort      ipcsum(uchar*);
Block*      ip4reassemble(IP*, int, Block*, Ip4hdr*);
void        ipfragfree4(IP*, Fragment4*);
Fragment4*  ipfragallo4(IP*);

/*s: function ip_init_6 */
void
ip_init_6(Fs *f)
{
    v6params *v6p;

    v6p = smalloc(sizeof(v6params));

    v6p->rp.mflag       = 0;        /* default not managed */
    v6p->rp.oflag       = 0;
    v6p->rp.maxraint    = 600000;   /* millisecs */
    v6p->rp.minraint    = 200000;
    v6p->rp.linkmtu     = 0;        /* no mtu sent */
    v6p->rp.reachtime   = 0;
    v6p->rp.rxmitra     = 0;
    v6p->rp.ttl     = MAXTTL;
    v6p->rp.routerlt    = 3 * v6p->rp.maxraint;

    v6p->hp.rxmithost   = 1000;     /* v6 RETRANS_TIMER */

    v6p->cdrouter       = -1;

    f->v6p          = v6p;
}
/*e: function ip_init_6 */

/*s: function initfrag */
void
initfrag(IP *ip, int size)
{
    Fragment4 *fq4, *eq4;
    /*s: [[initfrag()]] locals */
        Fragment6 *fq6, *eq6;
    /*e: [[initfrag()]] locals */

    ip->fragfree4 = (Fragment4*)malloc(sizeof(Fragment4) * size);
    if(ip->fragfree4 == nil)
        panic("initfrag");

    eq4 = &ip->fragfree4[size];
    for(fq4 = ip->fragfree4; fq4 < eq4; fq4++)
        fq4->next = fq4+1;
    ip->fragfree4[size-1].next = nil;

    /*s: [[initfrag()]] ipv6 init fragfree6 */
        ip->fragfree6 = (Fragment6*)malloc(sizeof(Fragment6) * size);
        if(ip->fragfree6 == nil)
            panic("initfrag");

        eq6 = &ip->fragfree6[size];
        for(fq6 = ip->fragfree6; fq6 < eq6; fq6++)
            fq6->next = fq6+1;

        ip->fragfree6[size-1].next = nil;
    /*e: [[initfrag()]] ipv6 init fragfree6 */
}
/*e: function initfrag */

/*s: function ip_init */
void
ip_init(Fs *f)
{
    IP *ip;

    ip = smalloc(sizeof(IP));
    initfrag(ip, 100);
    f->ip = ip;

    /*s: [[ip_init()]] ipv6 init */
    ip_init_6(f);
    /*e: [[ip_init()]] ipv6 init */
}
/*e: function ip_init */

/*s: function iprouting */
void
iprouting(Fs *f, bool on)
{
    f->ip->iprouting = on;
    if(f->ip->iprouting == false)
        f->ip->stats[Forwarding] = 2;
    else
        f->ip->stats[Forwarding] = 1;
}
/*e: function iprouting */

/*s: function ipoput4 */
int
ipoput4(Fs *f, Block *bp, bool gating, int ttl, int tos, Conv *c)
{
    IP *ip;
    Ip4hdr *eh;
    Ipifc *ifc;
    Route *r, *sr;
    uchar *gate;
    int len, medialen;
    int rv = OK_0;

    /*s: [[ipoput4()]] locals */
    Ip4hdr *feh;
    ulong fragoff;
    Block *xp, *nb;
    int lid, seglen, chunk, dlen, blklen, offset;
    /*e: [[ipoput4()]] locals */

    ip = f->ip;

    /* Fill out the ip header */
    eh = (Ip4hdr*)(bp->rp);

    ip->stats[OutRequests]++;

    /* Number of uchars in data and ip header to write */
    len = blocklen(bp);

    /*s: [[ipoput4()]] if gating */
    if(gating){
        chunk = nhgets(eh->length);
        if(chunk > len){
            ip->stats[OutDiscards]++;
            netlog(f, Logip, "short gated packet\n");
            goto free;
        }
        if(chunk < len)
            len = chunk;
    }
    /*e: [[ipoput4()]] if gating */

    /*s: [[ipoput4()]] error if too big packet of length len */
    if(len >= IP_MAX){
        ip->stats[OutDiscards]++;
        netlog(f, Logip, "exceeded ip max size %V\n", eh->dst);
        goto free;
    }
    /*e: [[ipoput4()]] error if too big packet of length len */

    // Finding the route for the destination!
    r = v4lookup(f, eh->dst, c);
    /*s: [[ipoput4()]] error if no route r */
    if(r == nil){
        ip->stats[OutNoRoutes]++;
        netlog(f, Logip, "no interface %V\n", eh->dst);
        rv = -1;
        goto free;
    }
    /*e: [[ipoput4()]] error if no route r */

    ifc = r->ifc;
    /*s: [[ipoput4()]] set gate according to type of route */
    if(r->type & (Rifc|Runi))
        gate = eh->dst;
    else
    /*s: [[ipoput4()]] adjust gate and ifc if broadcast or multicast case */
    if(r->type & (Rbcast|Rmulti)) {
        gate = eh->dst;
        sr = v4lookup(f, eh->src, nil);
        if(sr != nil && (sr->type & Runi))
            ifc = sr->ifc;
    }
    /*e: [[ipoput4()]] adjust gate and ifc if broadcast or multicast case */
    else
        gate = r->v4.gate;
    /*e: [[ipoput4()]] set gate according to type of route */

    if(!gating) {
        eh->vihl = IP_VER4|IP_HLEN4;
        eh->tos = tos;
    }
    eh->ttl = ttl;

    /*s: [[ipoput4()]] rlock ifc, goto free if cant */
    if(!canrlock(ifc))
        goto free;
    if(waserror()){
        runlock(ifc);
        nexterror();
    }
    /*e: [[ipoput4()]] rlock ifc, goto free if cant */
    /*s: [[ipoput4()]] error if no medium attached to interface ifc */
    if(ifc->m == nil)
        goto raise;
    /*e: [[ipoput4()]] error if no medium attached to interface ifc */

    /* If we dont need to fragment just send it */
    /*s: [[ipoput4()]] if manual fragmentation setting */
    if(c && c->maxfragsize && c->maxfragsize < ifc->maxtu)
        medialen = c->maxfragsize - ifc->m->hsize;
    /*e: [[ipoput4()]] if manual fragmentation setting */
    else
        medialen = ifc->maxtu - ifc->m->hsize;

    /*s: [[ipoput4()]] if no need to fragment, write simply to medium and return */
    if(len <= medialen) {
        if(!gating)
            hnputs(eh->id, incref(&ip->id4));
        hnputs(eh->length, len);
        // no fragment
        if(!gating){
            eh->frag[0] = 0;
            eh->frag[1] = 0;
        }
        eh->cksum[0] = 0;
        eh->cksum[1] = 0;
        hnputs(eh->cksum, ipcsum(&eh->vihl));
        assert(bp->next == nil);

        // Medium dispatch
        ifc->m->bwrite(ifc, bp, V4, gate);

        runlock(ifc);
        poperror();
        return OK_0;
    }
    /*e: [[ipoput4()]] if no need to fragment, write simply to medium and return */
    /*s: [[ipoput4()]] else, need to fragment */

    if(eh->frag[0] & (IP_DF>>8)){
        if (!gating) 
          print("%V: DF set\n", eh->dst);
        ip->stats[FragFails]++;
        ip->stats[OutDiscards]++;
        icmpcantfrag(f, bp, medialen);
        netlog(f, Logip, "%V: eh->frag[0] & (IP_DF>>8)\n", eh->dst);
        goto raise;
    }

    seglen = (medialen - IP4HDR) & ~7;
    if(seglen < 8){
        ip->stats[FragFails]++;
        ip->stats[OutDiscards]++;
        netlog(f, Logip, "%V seglen < 8\n", eh->dst);
        goto raise;
    }

    dlen = len - IP4HDR;
    xp = bp;
    if(gating)
        lid = nhgets(eh->id);
    else
        lid = incref(&ip->id4);

    offset = IP4HDR;
    while(xp != nil && offset && offset >= BLEN(xp)) {
        offset -= BLEN(xp);
        xp = xp->next;
    }
    xp->rp += offset;

    if(gating)
        fragoff = nhgets(eh->frag)<<3;
    else
        fragoff = 0;
    dlen += fragoff;

    for(; fragoff < dlen; fragoff += seglen) {
        nb = allocb(IP4HDR+seglen);
        feh = (Ip4hdr*)(nb->rp);

        memmove(nb->wp, eh, IP4HDR);
        nb->wp += IP4HDR;

        if((fragoff + seglen) >= dlen) {
            seglen = dlen - fragoff;
            hnputs(feh->frag, fragoff>>3);
        }
        else
            hnputs(feh->frag, (fragoff>>3)|IP_MF);

        hnputs(feh->length, seglen + IP4HDR);
        hnputs(feh->id, lid);

        /* Copy up the data area */
        chunk = seglen;
        while(chunk) {
            if(!xp) {
                ip->stats[OutDiscards]++;
                ip->stats[FragFails]++;
                freeblist(nb);
                netlog(f, Logip, "!xp: chunk %d\n", chunk);
                goto raise;
            }
            blklen = chunk;
            if(BLEN(xp) < chunk)
                blklen = BLEN(xp);
            memmove(nb->wp, xp->rp, blklen);
            nb->wp += blklen;
            xp->rp += blklen;
            chunk -= blklen;
            if(xp->rp == xp->wp)
                xp = xp->next;
        }

        feh->cksum[0] = 0;
        feh->cksum[1] = 0;
        hnputs(feh->cksum, ipcsum(&feh->vihl));

        // Medium dispatch, send this fragment
        ifc->m->bwrite(ifc, nb, V4, gate);

        ip->stats[FragCreates]++;
    }
    ip->stats[FragOKs]++;
    /*e: [[ipoput4()]] else, need to fragment */

raise:
    runlock(ifc);
    poperror();
free:
    freeblist(bp);
    return rv;
}
/*e: function ipoput4 */

/*s: function ipiput4 */
void
ipiput4(Fs *f, Ipifc *ifc, Block *bp)
{
    IP *ip;
    Ip4hdr *h;
    uchar v6dst[IPaddrlen];
    // enum<protocol_type>
    int proto;
    Proto *p;
    bool notforme;
    /*s: [[ipiput4()]] locals */
    int hl;
    int olen;
    uchar *dp;
    /*x: [[ipiput4()]] locals */
    ushort frag;
    /*x: [[ipiput4()]] locals */
    int hop, tos;
    Route *r;
    Conv conv;
    /*e: [[ipiput4()]] locals */

    /*s: [[ipiput4()]] call ipiput6 if block is not ipv4 */
    if(BLKIPVER(bp) != IP_VER4) {
        ipiput6(f, ifc, bp);
        return;
    }
    /*e: [[ipiput4()]] call ipiput6 if block is not ipv4 */

    ip = f->ip;
    ip->stats[InReceives]++;

    /*s: [[ipiput4()]] ensure we have all the header in the first block */
    /*
     *  Ensure we have all the header info in the first
     *  block.  Make life easier for other protocols by
     *  collecting up to the first 64 bytes in the first block.
     */
    if(BLEN(bp) < 64) {
        hl = blocklen(bp);
        if(hl < IP4HDR)
            hl = IP4HDR;
        if(hl > 64)
            hl = 64;
        bp = pullupblock(bp, hl);
        if(bp == nil)
            return;
    }
    /*e: [[ipiput4()]] ensure we have all the header in the first block */

    h = (Ip4hdr*)(bp->rp);

    /*s: [[ipiput4()]] check header checksum */
    /* dump anything that whose header doesn't checksum */
    if((bp->flag & Bipck) == 0 && ipcsum(&h->vihl)) {
        ip->stats[InHdrErrors]++;
        netlog(f, Logip, "ip: checksum error %V\n", h->src);
        freeblist(bp);
        return;
    }
    /*e: [[ipiput4()]] check header checksum */

    v4tov6(v6dst, h->dst);
    notforme = ipforme(f, v6dst) == 0;

    /*s: [[ipiput4()]] check header length and version */
    /* Check header length and version */
    if((h->vihl&0x0F) != IP_HLEN4) {
        hl = (h->vihl&0xF)<<2;
        if(hl < (IP_HLEN4<<2)) {
            ip->stats[InHdrErrors]++;
            netlog(f, Logip, "ip: %V bad hivl %ux\n", h->src, h->vihl);
            freeblist(bp);
            return;
        }
        /* If this is not routed strip off the options */
        if(notforme == false) {
            olen = nhgets(h->length);
            dp = bp->rp + (hl - (IP_HLEN4<<2));
            memmove(dp, h, IP_HLEN4<<2);
            bp->rp = dp;
            h = (Ip4hdr*)(bp->rp);
            h->vihl = (IP_VER4|IP_HLEN4);
            hnputs(h->length, olen-hl+(IP_HLEN4<<2));
        }
    }
    /*e: [[ipiput4()]] check header length and version */

    /*s: [[ipiput4()]] if notforme */
        /* route */
        if(notforme) {
            if(!ip->iprouting){
                freeblist(bp);
                return;
            }

            /* don't forward to source's network */
            memset(&conv, 0, sizeof conv);
            conv.r = nil;
            r = v4lookup(f, h->dst, &conv);
            if(r == nil || r->ifc == ifc){
                ip->stats[OutDiscards]++;
                freeblist(bp);
                return;
            }

            /* don't forward if packet has timed out */
            hop = h->ttl;
            if(hop < 1) {
                ip->stats[InHdrErrors]++;
                icmpttlexceeded(f, ifc->lifc->local, bp);
                freeblist(bp);
                return;
            }

            /* reassemble if the interface expects it */
            if(r->ifc == nil) panic("nil route rfc");
            if(r->ifc->reassemble){
                frag = nhgets(h->frag);
                if(frag) {
                    h->tos = 0;
                    if(frag & IP_MF)
                        h->tos = 1;
                    bp = ip4reassemble(ip, frag, bp, h);
                    if(bp == nil)
                        return;
                    h = (Ip4hdr*)(bp->rp);
                }
            }

            ip->stats[ForwDatagrams]++;
            tos = h->tos;
            hop = h->ttl;

            ipoput4(f, bp, true, hop - 1, tos, &conv);

            return;
        }
    /*e: [[ipiput4()]] if notforme */
    // else have a ipforme

    /*s: [[ipiput4()]] possibly defragment and reassemble */
    frag = nhgets(h->frag);
    if(frag) {
        h->tos = 0;
        if(frag & IP_MF)
            h->tos = 1;
        bp = ip4reassemble(ip, frag, bp, h);
        if(bp == nil)
            return;
        h = (Ip4hdr*)(bp->rp);
    }
    /*e: [[ipiput4()]] possibly defragment and reassemble */

    /* don't let any frag info go up the stack */
    h->frag[0] = 0;
    h->frag[1] = 0;

    proto = h->proto;
    p = Fsrcvpcol(f, proto);

    if(p != nil && p->rcv != nil) {
        ip->stats[InDelivers]++;

        // Protocol dispatch
        (*p->rcv)(p, ifc, bp);
        return;
    }

    ip->stats[InDiscards]++;
    ip->stats[InUnknownProtos]++;
    freeblist(bp);
}
/*e: function ipiput4 */

/*s: function ipstats */
int
ipstats(Fs *f, char *buf, int len)
{
    IP *ip;
    char *p, *e;
    int i;

    ip = f->ip;
    ip->stats[DefaultTTL] = MAXTTL;

    p = buf;
    e = p+len;
    for(i = 0; i < Nipstats; i++)
        p = seprint(p, e, "%s: %llud\n", statnames[i], ip->stats[i]);
    return p - buf;
}
/*e: function ipstats */

/*s: function ip4reassemble */
Block*
ip4reassemble(IP *ip, int offset, Block *bp, Ip4hdr *ih)
{
    int fend;
    ushort id;
    Fragment4 *f, *fnext;
    iplong src, dst;
    Block *bl, **l, *last, *prev;
    int ovlap, len, fragsize, pktposn;

    src = nhgetl(ih->src);
    dst = nhgetl(ih->dst);
    id = nhgets(ih->id);

    /*
     *  block lists are too hard, pullupblock into a single block
     */
    if(bp->next){
        bp = pullupblock(bp, blocklen(bp));
        ih = (Ip4hdr*)(bp->rp);
    }

    qlock(&ip->fraglock4);

    /*
     *  find a reassembly queue for this fragment
     */
    for(f = ip->flisthead4; f; f = fnext){
        fnext = f->next;    /* because ipfragfree4 changes the list */
        if(f->src == src && f->dst == dst && f->id == id)
            break;
        if(f->age < NOW){
            ip->stats[ReasmTimeout]++;
            ipfragfree4(ip, f);
        }
    }

    /*
     *  if this isn't a fragmented packet, accept it
     *  and get rid of any fragments that might go
     *  with it.
     */
    if(!ih->tos && (offset & ~(IP_MF|IP_DF)) == 0) {
        if(f != nil) {
            ipfragfree4(ip, f);
            ip->stats[ReasmFails]++;
        }
        qunlock(&ip->fraglock4);
        return bp;
    }

    if(bp->base+IPFRAGSZ >= bp->rp){
        bp = padblock(bp, IPFRAGSZ);
        bp->rp += IPFRAGSZ;
    }

    BKFG(bp)->foff = offset<<3;
    BKFG(bp)->flen = nhgets(ih->length)-IP4HDR;

    /* First fragment allocates a reassembly queue */
    if(f == nil) {
        f = ipfragallo4(ip);
        f->id = id;
        f->src = src;
        f->dst = dst;

        f->blist = bp;

        qunlock(&ip->fraglock4);
        ip->stats[ReasmReqds]++;
        return nil;
    }

    /*
     *  find the new fragment's position in the queue
     */
    prev = nil;
    l = &f->blist;
    bl = f->blist;
    while(bl != nil && BKFG(bp)->foff > BKFG(bl)->foff) {
        prev = bl;
        l = &bl->next;
        bl = bl->next;
    }

    /* Check overlap of a previous fragment - trim away as necessary */
    if(prev) {
        ovlap = BKFG(prev)->foff + BKFG(prev)->flen - BKFG(bp)->foff;
        if(ovlap > 0) {
            if(ovlap >= BKFG(bp)->flen) {
                freeblist(bp);
                qunlock(&ip->fraglock4);
                return nil;
            }
            BKFG(prev)->flen -= ovlap;
        }
    }

    /* Link onto assembly queue */
    bp->next = *l;
    *l = bp;

    /* Check to see if succeeding segments overlap */
    if(bp->next) {
        l = &bp->next;
        fend = BKFG(bp)->foff + BKFG(bp)->flen;
        /* Take completely covered segments out */
        while(*l) {
            ovlap = fend - BKFG(*l)->foff;
            if(ovlap <= 0)
                break;
            if(ovlap < BKFG(*l)->flen) {
                BKFG(*l)->flen -= ovlap;
                BKFG(*l)->foff += ovlap;
                /* move up ih hdrs */
                memmove((*l)->rp + ovlap, (*l)->rp, IP4HDR);
                (*l)->rp += ovlap;
                break;
            }
            last = (*l)->next;
            (*l)->next = nil;
            freeblist(*l);
            *l = last;
        }
    }

    /*
     *  look for a complete packet.  if we get to a fragment
     *  without IP_MF set, we're done.
     */
    pktposn = 0;
    for(bl = f->blist; bl; bl = bl->next) {
        if(BKFG(bl)->foff != pktposn)
            break;
        if((BLKIP(bl)->frag[0]&(IP_MF>>8)) == 0) {
            bl = f->blist;
            len = nhgets(BLKIP(bl)->length);
            bl->wp = bl->rp + len;

            /* Pullup all the fragment headers and
             * return a complete packet
             */
            for(bl = bl->next; bl; bl = bl->next) {
                fragsize = BKFG(bl)->flen;
                len += fragsize;
                bl->rp += IP4HDR;
                bl->wp = bl->rp + fragsize;
            }

            bl = f->blist;
            f->blist = nil;
            ipfragfree4(ip, f);
            ih = BLKIP(bl);
            hnputs(ih->length, len);
            qunlock(&ip->fraglock4);
            ip->stats[ReasmOKs]++;
            return bl;
        }
        pktposn += BKFG(bl)->flen;
    }
    qunlock(&ip->fraglock4);
    return nil;
}
/*e: function ip4reassemble */

/*s: function ipfragfree4 */
/*
 * ipfragfree4 - Free a list of fragments - assume hold fraglock4
 */
void
ipfragfree4(IP *ip, Fragment4 *frag)
{
    Fragment4 *fl, **l;

    if(frag->blist)
        freeblist(frag->blist);

    frag->src = 0;
    frag->id = 0;
    frag->blist = nil;

    l = &ip->flisthead4;
    for(fl = *l; fl; fl = fl->next) {
        if(fl == frag) {
            *l = frag->next;
            break;
        }
        l = &fl->next;
    }

    frag->next = ip->fragfree4;
    ip->fragfree4 = frag;

}
/*e: function ipfragfree4 */

/*s: function ipfragallo4 */
/*
 * ipfragallo4 - allocate a reassembly queue - assume hold fraglock4
 */
Fragment4 *
ipfragallo4(IP *ip)
{
    Fragment4 *f;

    while(ip->fragfree4 == nil) {
        /* free last entry on fraglist */
        for(f = ip->flisthead4; f->next; f = f->next)
            ;
        ipfragfree4(ip, f);
    }
    f = ip->fragfree4;
    ip->fragfree4 = f->next;
    f->next = ip->flisthead4;
    ip->flisthead4 = f;
    f->age = NOW + 30000;

    return f;
}
/*e: function ipfragallo4 */

/*s: function ipcsum */
ushort
ipcsum(uchar *addr)
{
    int len;
    ulong sum;

    sum = 0;
    len = (addr[0]&0xf)<<2;

    while(len > 0) {
        sum += addr[0]<<8 | addr[1] ;
        len -= 2;
        addr += 2;
    }

    sum = (sum & 0xffff) + (sum >> 16);
    sum = (sum & 0xffff) + (sum >> 16);

    return (sum^0xffff);
}
/*e: function ipcsum */
/*e: kernel/network/ip/ip.c */
