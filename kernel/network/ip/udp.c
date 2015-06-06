/*s: kernel/network/ip/udp.c */
#include    "u.h"
#include    "../port/lib.h"
#include    "mem.h"
#include    "dat.h"
#include    "fns.h"
#include    "../port/error.h"

#include    "ip.h"
#include    "ipv6.h"


/*s: constant DPRINT */
#define DPRINT if(0)print
/*e: constant DPRINT */

/*s: enum _anon_ (kernel/network/ip/udp.c) */
enum
{
    /*s: constant UDP_UDPHDR_SZ */
    UDP_UDPHDR_SZ   = 8,
    /*e: constant UDP_UDPHDR_SZ */

    /*s: constant UDP4_PHDR_OFF */
    UDP4_PHDR_OFF = 8,
    /*e: constant UDP4_PHDR_OFF */
    /*s: constant UDP4_PHDR_SZ */
    UDP4_PHDR_SZ = 12,
    /*e: constant UDP4_PHDR_SZ */
    /*s: constant UDP4_IPHDR_SZ */
    UDP4_IPHDR_SZ = 20,
    /*e: constant UDP4_IPHDR_SZ */

    /*s: constant UDP6_xxx */
    UDP6_IPHDR_SZ = 40,
    UDP6_PHDR_SZ = 40,
    UDP6_PHDR_OFF = 0,
    /*e: constant UDP6_xxx */
    /*s: constant IP_UDPPROTO */
    IP_UDPPROTO = 17,
    /*x: constant IP_UDPPROTO */
    #define IP_UDPPROTO	17
    /*e: constant IP_UDPPROTO */

    /*s: constant UDP_USEAD7 */
    UDP_USEAD7  = 52,
    /*e: constant UDP_USEAD7 */

    Udprxms     = 200,
    Udptickms   = 100,
    Udpmaxxmit  = 10,
};
/*e: enum _anon_ (kernel/network/ip/udp.c) */

// forward decl
typedef struct Udp4hdr Udp4hdr;
typedef struct Udp6hdr Udp6hdr;
typedef struct Udpstats Udpstats;
typedef struct Udppriv Udppriv;
typedef struct Udpcb Udpcb;

//void (*etherprofiler)(char *name, int qlen);
void udpkick(void *x, Block *bp);


/*s: struct Udp4hdr */
struct Udp4hdr
{
    /* ip header */
    uchar   vihl;       /* Version and header length */
    uchar   tos;        /* Type of service */
    uchar   length[2];  /* packet length */
    uchar   id[2];      /* Identification */
    uchar   frag[2];    /* Fragment information */
    uchar   Unused; // ttl

    uchar   udpproto;   /* Protocol */
    uchar   udpplen[2]; /* Header plus data length */
    uchar   udpsrc[IPv4addrlen];    /* Ip source */
    uchar   udpdst[IPv4addrlen];    /* Ip destination */

    /* udp header */
    uchar   udpsport[2];    /* Source port */
    uchar   udpdport[2];    /* Destination port */
    uchar   udplen[2];  /* data length */
    uchar   udpcksum[2];    /* Checksum */
};
/*e: struct Udp4hdr */

/*s: struct Udp6hdr */
struct Udp6hdr {
    uchar viclfl[4];
    uchar len[2];
    uchar nextheader;
    uchar hoplimit;
    uchar udpsrc[IPaddrlen];
    uchar udpdst[IPaddrlen];

    /* udp header */
    uchar   udpsport[2];    /* Source port */
    uchar   udpdport[2];    /* Destination port */
    uchar   udplen[2];  /* data length */
    uchar   udpcksum[2];    /* Checksum */
};
/*e: struct Udp6hdr */

/*s: struct Udpstats */
/* MIB II counters */
struct Udpstats
{
    uvlong  udpInDatagrams;
    ulong   udpNoPorts;
    ulong   udpInErrors;
    uvlong  udpOutDatagrams;
};
/*e: struct Udpstats */

/*s: struct Udppriv */
struct Udppriv
{
    Ipht        ht;

    /*s: [[Udppriv]] stat fields */
    /* MIB counters */
    Udpstats    ustats;
    /* non-MIB stats */
    ulong       csumerr;        /* checksum errors */
    ulong       lenerr;         /* short packet */
    /*e: [[Udppriv]] stat fields */
};
/*e: struct Udppriv */

/*
 *  protocol specific part of Conv
 */
/*s: struct Udpcb */
struct Udpcb
{
    QLock;
    /*s: [[Idpcb]] other fields */
    uchar   headers;
    /*e: [[Idpcb]] other fields */
};
/*e: struct Udpcb */

/*s: function udpconnect */
static char*
udpconnect(Conv *c, char **argv, int argc)
{
    char *err;
    Udppriv *upriv;

    upriv = c->p->priv;

    err = Fsstdconnect(c, argv, argc);
    Fsconnected(c, err);
    if(err != nil)
        return err;

    iphtadd(&upriv->ht, c);
    return nil;
}
/*e: function udpconnect */


/*s: function udpstate */
static int
udpstate(Conv *cv, char *state, int n)
{
    return snprint(state, n, "%s qin %d qout %d\n",
        cv->inuse ? "Open" : "Closed",
        cv->rq ? qlen(cv->rq) : 0,
        cv->wq ? qlen(cv->wq) : 0
    );
}
/*e: function udpstate */

/*s: function udpannounce */
static char*
udpannounce(Conv *c, char** argv, int argc)
{
    char *err;
    Udppriv *upriv;

    upriv = c->p->priv;

    err = Fsstdannounce(c, argv, argc);
    if(err != nil)
        return err;
    Fsconnected(c, nil);

    iphtadd(&upriv->ht, c);

    return nil;
}
/*e: function udpannounce */

/*s: function udpcreate */
static void
udpcreate(Conv *cv)
{
    cv->rq = qopen(128*1024, Qmsg, 0, 0);
    cv->wq = qbypass(udpkick, cv);
}
/*e: function udpcreate */

/*s: function udpclose */
static void
udpclose(Conv *c)
{
    Udpcb *ucb;
    Udppriv *upriv;

    upriv = c->p->priv;
    iphtrem(&upriv->ht, c);

    c->state = Idle;
    qclose(c->rq);
    qclose(c->wq);
    qclose(c->eq);
    ipmove(c->laddr, IPnoaddr);
    ipmove(c->raddr, IPnoaddr);
    c->lport = 0;
    c->rport = 0;

    ucb = (Udpcb*)c->ptcl;
    ucb->headers = 0;
}
/*e: function udpclose */

/*s: function udpkick */
void
udpkick(void *x, Block *bp)
{
    Conv *c = x;
    Conv *rc;
    Udp4hdr *uh4;
    Udppriv *upriv;
    int dlen, ptcllen;
    Fs *f;
    int version;
    /*s: [[udpkick()]] locals */
    Udpcb *ucb;
    uchar laddr[IPaddrlen];
    uchar raddr[IPaddrlen];
    ushort rport;
    /*x: [[udpkick()]] locals */
    Udp6hdr *uh6;
    /*e: [[udpkick()]] locals */

    upriv = c->p->priv;
    f = c->p->f;

//  netlog(c->p->f, Logudp, "udp: kick\n"); /* frequent and uninteresting */
    if(bp == nil)
        return;

    /*s: [[udpkick()]] special headers processing */
    ucb = (Udpcb*)c->ptcl;
    switch(ucb->headers) {
    case 7:
        /* get user specified addresses */
        bp = pullupblock(bp, UDP_USEAD7);
        if(bp == nil)
            return;
        ipmove(raddr, bp->rp);
        bp->rp += IPaddrlen;
        ipmove(laddr, bp->rp);
        bp->rp += IPaddrlen;

        /* pick interface closest to dest */
        if(ipforme(f, laddr) != Runi)
            findlocalip(f, laddr, raddr);
        bp->rp += IPaddrlen;        /* Ignore ifc address */
        rport = nhgets(bp->rp);
        bp->rp += 2+2;          /* Ignore local port */
        break;
    default:
        rport = 0;
        break;
    }
    /*e: [[udpkick()]] special headers processing */
    /*s: [[udpkick()]] set version to V4 or V6 */
    /*s: [[udpkick()]] set version to V4 or V6 if special headers */
    if(ucb->headers) {
        if(memcmp(laddr, v4prefix, IPv4off) == 0
        || ipcmp(laddr, IPnoaddr) == 0)
            version = V4;
        else
            version = V6;
    }
    /*e: [[udpkick()]] set version to V4 or V6 if special headers */
    else {
        if( (memcmp(c->raddr, v4prefix, IPv4off) == 0 &&
            memcmp(c->laddr, v4prefix, IPv4off) == 0)
            || ipcmp(c->raddr, IPnoaddr) == 0)
            version = V4;
        else
            version = V6;
    }
    /*e: [[udpkick()]] set version to V4 or V6 */

    dlen = blocklen(bp);

    /* fill in pseudo header and compute checksum */
    switch(version){
    case V4:
        bp = padblock(bp, UDP4_IPHDR_SZ+UDP_UDPHDR_SZ);
        if(bp == nil)
            return;

        uh4 = (Udp4hdr *)(bp->rp);
        ptcllen = dlen + UDP_UDPHDR_SZ;
        uh4->Unused = 0;
        uh4->udpproto = IP_UDPPROTO;
        uh4->frag[0] = 0;
        uh4->frag[1] = 0;
        hnputs(uh4->udpplen, ptcllen);
        /*s: [[udpkick()]] if special headers */
        if(ucb->headers) {
            v6tov4(uh4->udpdst, raddr);
            hnputs(uh4->udpdport, rport);
            v6tov4(uh4->udpsrc, laddr);
            rc = nil;
        } 
        /*e: [[udpkick()]] if special headers */
        else {
            v6tov4(uh4->udpdst, c->raddr);
            hnputs(uh4->udpdport, c->rport);
            if(ipcmp(c->laddr, IPnoaddr) == 0)
                findlocalip(f, c->laddr, c->raddr);
            v6tov4(uh4->udpsrc, c->laddr);
            rc = c;
        }
        hnputs(uh4->udpsport, c->lport);
        hnputs(uh4->udplen, ptcllen);

        uh4->udpcksum[0] = 0;
        uh4->udpcksum[1] = 0;
        hnputs(uh4->udpcksum,
               ptclcsum(bp, UDP4_PHDR_OFF, dlen+UDP_UDPHDR_SZ+UDP4_PHDR_SZ));
        uh4->vihl = IP_VER4;

        // Let's go, let's send the data
        ipoput4(f, bp, false, c->ttl, c->tos, rc);

        break;

    /*s: [[udpkick()]] switch version ipv6 case */
    case V6:
        bp = padblock(bp, UDP6_IPHDR_SZ+UDP_UDPHDR_SZ);
        if(bp == nil)
            return;

        /*
         * using the v6 ip header to create pseudo header
         * first then reset it to the normal ip header
         */
        uh6 = (Udp6hdr *)(bp->rp);
        memset(uh6, 0, 8);
        ptcllen = dlen + UDP_UDPHDR_SZ;
        hnputl(uh6->viclfl, ptcllen);
        uh6->hoplimit = IP_UDPPROTO;
        if(ucb->headers) {
            ipmove(uh6->udpdst, raddr);
            hnputs(uh6->udpdport, rport);
            ipmove(uh6->udpsrc, laddr);
            rc = nil;
        } else {
            ipmove(uh6->udpdst, c->raddr);
            hnputs(uh6->udpdport, c->rport);
            if(ipcmp(c->laddr, IPnoaddr) == 0)
                findlocalip(f, c->laddr, c->raddr);
            ipmove(uh6->udpsrc, c->laddr);
            rc = c;
        }
        hnputs(uh6->udpsport, c->lport);
        hnputs(uh6->udplen, ptcllen);
        uh6->udpcksum[0] = 0;
        uh6->udpcksum[1] = 0;
        hnputs(uh6->udpcksum,
               ptclcsum(bp, UDP6_PHDR_OFF, dlen+UDP_UDPHDR_SZ+UDP6_PHDR_SZ));
        memset(uh6, 0, 8);
        uh6->viclfl[0] = IP_VER6;
        hnputs(uh6->len, ptcllen);
        uh6->nextheader = IP_UDPPROTO;
        ipoput6(f, bp, 0, c->ttl, c->tos, rc);
        break;

    /*e: [[udpkick()]] switch version ipv6 case */

    default:
        panic("udpkick: version %d", version);
    }
    upriv->ustats.udpOutDatagrams++;
}
/*e: function udpkick */

/*s: function udpiput */
void
udpiput(Proto *udp, Ipifc *ifc, Block *bp)
{
    int len;
    Udp4hdr *uh4;
    Conv *c;

    Udpcb *ucb;
    uchar raddr[IPaddrlen], laddr[IPaddrlen];
    ushort rport, lport;

    Udp6hdr *uh6;

    Udppriv *upriv;
    Fs *f;
    int version;
    int ottl, oviclfl, olen;
    uchar *p;

    upriv = udp->priv;
    f = udp->f;
    upriv->ustats.udpInDatagrams++;

    uh4 = (Udp4hdr*)(bp->rp);
    version = ((uh4->vihl&0xF0)==IP_VER6) ? V6 : V4;

    /* Put back pseudo header for checksum
     * (remember old values for icmpnoconv()) */
    switch(version) {
    case V4:
        ottl = uh4->Unused;
        uh4->Unused = 0;
        len = nhgets(uh4->udplen);
        olen = nhgets(uh4->udpplen);
        hnputs(uh4->udpplen, len);

        v4tov6(raddr, uh4->udpsrc);
        v4tov6(laddr, uh4->udpdst);
        lport = nhgets(uh4->udpdport);
        rport = nhgets(uh4->udpsport);

        if(nhgets(uh4->udpcksum)) {
            if(ptclcsum(bp, UDP4_PHDR_OFF, len+UDP4_PHDR_SZ)) {
                upriv->ustats.udpInErrors++;
                netlog(f, Logudp, "udp: checksum error %I\n", raddr);
                DPRINT("udp: checksum error %I\n", raddr);
                freeblist(bp);
                return;
            }
        }
        uh4->Unused = ottl;
        hnputs(uh4->udpplen, olen);
        break;

    case V6:
        uh6 = (Udp6hdr*)(bp->rp);
        len = nhgets(uh6->udplen);
        oviclfl = nhgetl(uh6->viclfl);
        olen = nhgets(uh6->len);
        ottl = uh6->hoplimit;
        ipmove(raddr, uh6->udpsrc);
        ipmove(laddr, uh6->udpdst);
        lport = nhgets(uh6->udpdport);
        rport = nhgets(uh6->udpsport);
        memset(uh6, 0, 8);
        hnputl(uh6->viclfl, len);
        uh6->hoplimit = IP_UDPPROTO;
        if(ptclcsum(bp, UDP6_PHDR_OFF, len+UDP6_PHDR_SZ)) {
            upriv->ustats.udpInErrors++;
            netlog(f, Logudp, "udp: checksum error %I\n", raddr);
            DPRINT("udp: checksum error %I\n", raddr);
            freeblist(bp);
            return;
        }
        hnputl(uh6->viclfl, oviclfl);
        hnputs(uh6->len, olen);
        uh6->nextheader = IP_UDPPROTO;
        uh6->hoplimit = ottl;
        break;
    default:
        panic("udpiput: version %d", version);
        return; /* to avoid a warning */
    }

    qlock(udp);

    c = iphtlook(&upriv->ht, raddr, rport, laddr, lport);
    if(c == nil){
        /* no conversation found */
        upriv->ustats.udpNoPorts++;
        qunlock(udp);
        netlog(f, Logudp, "udp: no conv %I!%d -> %I!%d\n", raddr, rport,
               laddr, lport);

        switch(version){
        case V4:
            icmpnoconv(f, bp);
            break;
        case V6:
            icmphostunr(f, ifc, bp, Icmp6_port_unreach, 0);
            break;
        default:
            panic("udpiput2: version %d", version);
        }

        freeblist(bp);
        return;
    }
    ucb = (Udpcb*)c->ptcl;

    if(c->state == Announced){
        if(ucb->headers == 0){
            /* create a new conversation */
            if(ipforme(f, laddr) != Runi) {
                switch(version){
                case V4:
                    v4tov6(laddr, ifc->lifc->local);
                    break;
                case V6:
                    ipmove(laddr, ifc->lifc->local);
                    break;
                default:
                    panic("udpiput3: version %d", version);
                }
            }
            c = Fsnewcall(c, raddr, rport, laddr, lport, version);
            if(c == nil){
                qunlock(udp);
                freeblist(bp);
                return;
            }
            iphtadd(&upriv->ht, c);
            ucb = (Udpcb*)c->ptcl;
        }
    }

    qlock(c);
    qunlock(udp);

    /*
     * Trim the packet down to data size
     */
    len -= UDP_UDPHDR_SZ;
    switch(version){
    case V4:
        bp = trimblock(bp, UDP4_IPHDR_SZ+UDP_UDPHDR_SZ, len);
        break;
    case V6:
        bp = trimblock(bp, UDP6_IPHDR_SZ+UDP_UDPHDR_SZ, len);
        break;
    default:
        bp = nil;
        panic("udpiput4: version %d", version);
    }
    if(bp == nil){
        qunlock(c);
        netlog(f, Logudp, "udp: len err %I.%d -> %I.%d\n", raddr, rport,
               laddr, lport);
        upriv->lenerr++;
        return;
    }

    netlog(f, Logudpmsg, "udp: %I.%d -> %I.%d l %d\n", raddr, rport,
           laddr, lport, len);

    switch(ucb->headers){
    case 7:
        /* pass the src address */
        bp = padblock(bp, UDP_USEAD7);
        p = bp->rp;
        ipmove(p, raddr); p += IPaddrlen;
        ipmove(p, laddr); p += IPaddrlen;
        ipmove(p, ifc->lifc->local); p += IPaddrlen;
        hnputs(p, rport); p += 2;
        hnputs(p, lport);
        break;
    }

    if(bp->next)
        bp = concatblock(bp);

    if(qfull(c->rq)){
        qunlock(c);
        netlog(f, Logudp, "udp: qfull %I.%d -> %I.%d\n", raddr, rport,
               laddr, lport);
        freeblist(bp);
        return;
    }

    qpass(c->rq, bp);
    qunlock(c);

}
/*e: function udpiput */

/*s: function udpctl */
char*
udpctl(Conv *c, char **f, int n)
{
    Udpcb *ucb;

    ucb = (Udpcb*)c->ptcl;
    if(n == 1){
        if(strcmp(f[0], "headers") == 0){
            ucb->headers = 7;   /* new headers format */
            return nil;
        }
    }
    return "unknown control request";
}
/*e: function udpctl */

/*s: function udpadvise */
void
udpadvise(Proto *udp, Block *bp, char *msg)
{
    Udp4hdr *h4;
    Udp6hdr *h6;
    uchar source[IPaddrlen], dest[IPaddrlen];
    ushort psource, pdest;
    Conv *s, **p;
    int version;

    h4 = (Udp4hdr*)(bp->rp);
    version = ((h4->vihl&0xF0)==IP_VER6) ? 6 : 4;

    switch(version) {
    case V4:
        v4tov6(dest, h4->udpdst);
        v4tov6(source, h4->udpsrc);
        psource = nhgets(h4->udpsport);
        pdest = nhgets(h4->udpdport);
        break;
    case V6:
        h6 = (Udp6hdr*)(bp->rp);
        ipmove(dest, h6->udpdst);
        ipmove(source, h6->udpsrc);
        psource = nhgets(h6->udpsport);
        pdest = nhgets(h6->udpdport);
        break;
    default:
        panic("udpadvise: version %d", version);
        return;  /* to avoid a warning */
    }

    /* Look for a connection */
    qlock(udp);
    for(p = udp->conv; *p; p++) {
        s = *p;
        if(s->rport == pdest)
        if(s->lport == psource)
        if(ipcmp(s->raddr, dest) == 0)
        if(ipcmp(s->laddr, source) == 0){
            if(s->ignoreadvice)
                break;
            qlock(s);
            qunlock(udp);
            qhangup(s->rq, msg);
            qhangup(s->wq, msg);
            qunlock(s);
            freeblist(bp);
            return;
        }
    }
    qunlock(udp);
    freeblist(bp);
}
/*e: function udpadvise */

/*s: function udpstats */
int
udpstats(Proto *udp, char *buf, int len)
{
    Udppriv *upriv;

    upriv = udp->priv;
    return snprint(buf, len, "InDatagrams: %llud\nNoPorts: %lud\n"
        "InErrors: %lud\nOutDatagrams: %llud\n",
        upriv->ustats.udpInDatagrams,
        upriv->ustats.udpNoPorts,
        upriv->ustats.udpInErrors,
        upriv->ustats.udpOutDatagrams);
}
/*e: function udpstats */

/*s: function udpinit */
void
udpinit(Fs *fs)
{
    Proto *udp;

    udp = smalloc(sizeof(Proto));
    udp->priv = smalloc(sizeof(Udppriv));

    udp->name = "udp";
    udp->create = udpcreate;
    udp->close = udpclose;

    udp->connect = udpconnect;
    udp->announce = udpannounce;
    udp->ctl = udpctl;

    udp->rcv = udpiput;

    udp->state = udpstate;
    udp->stats = udpstats;

    udp->advise = udpadvise;

    udp->ipproto = IP_UDPPROTO;

    udp->nc = Nchans;
    udp->ptclsize = sizeof(Udpcb);

    Fsproto(fs, udp);
}
/*e: function udpinit */
/*e: kernel/network/ip/udp.c */
