/*s: networking/ip/ping.c */
/* ping for ip v4 and v6 */
#include <u.h>
#include <libc.h>
#include <ctype.h>
#include <ip.h>
#include <bio.h>
#include <ndb.h>
#include "icmp.h"

/*s: enum [[_anon_ (networking/ip/ping.c)]] */
enum {
    MAXMSG		= 32,
    SLEEPMS		= 1000,

    SECOND		= 1000000000LL,
    MINUTE		= 60*SECOND,
};
/*e: enum [[_anon_ (networking/ip/ping.c)]] */

typedef struct Req Req;
/*s: struct [[Req]] */
struct Req
{
    ushort	seq;	/* sequence number */
    vlong	time;	/* time sent */
    vlong	rtt;
    int	ttl;
    int	replied;
    Req	 *next;
};
/*e: struct [[Req]] */

typedef struct {
    int	version;
    char	*net;
    int	echocmd;
    int	echoreply;
    unsigned iphdrsz;

    void	(*prreply)(Req *r, void *v);
    void	(*prlost)(ushort seq, void *v);
} Proto;


/*s: global [[first]] */
Req	*first;		/* request list */
/*e: global [[first]] */
/*s: global [[last]] */
Req	*last;		/* ... */
/*e: global [[last]] */
/*s: global [[listlock]] */
Lock	listlock;
/*e: global [[listlock]] */

/*s: global [[argv0]] */
char *argv0;
/*e: global [[argv0]] */

/*s: global [[addresses]] */
int addresses;
/*e: global [[addresses]] */
/*s: global [[debug]] */
int debug;
/*e: global [[debug]] */
/*s: global [[done]] */
int done;
/*e: global [[done]] */
/*s: global [[flood]] */
int flood;
/*e: global [[flood]] */
/*s: global [[lostmsgs]] */
int lostmsgs;
/*e: global [[lostmsgs]] */
/*s: global [[lostonly]] */
int lostonly;
/*e: global [[lostonly]] */
/*s: global [[quiet]] */
int quiet;
/*e: global [[quiet]] */
/*s: global [[rcvdmsgs]] */
int rcvdmsgs;
/*e: global [[rcvdmsgs]] */
/*s: global [[rint]] */
int rint;
/*e: global [[rint]] */
/*s: global [[firstseq]] */
ushort firstseq;
/*e: global [[firstseq]] */
/*s: global [[sum]] */
vlong sum;
/*e: global [[sum]] */
/*s: global [[waittime]] */
int waittime = 5000;
/*e: global [[waittime]] */

static char *network, *target;

void lost(Req*, void*);
void reply(Req*, void*);

/*s: function [[usage]] */
static void
usage(void)
{
    fprint(2,
        "usage: %s [-6alq] [-s msgsize] [-i millisecs] [-n #pings] dest\n",
        argv0);
    exits("usage");
}
/*e: function [[usage]] */

/*s: function [[catch]] */
static void
catch(void *a, char *msg)
{
    USED(a);
    if(strstr(msg, "alarm"))
        noted(NCONT);
    else if(strstr(msg, "die"))
        exits("errors");
    else
        noted(NDFLT);
}
/*e: function [[catch]] */

/*s: function [[prlost4]] */
static void
prlost4(ushort seq, void *v)
{
    Ip4hdr *ip4 = v;

    print("lost %ud: %V -> %V\n", seq, ip4->src, ip4->dst);
}
/*e: function [[prlost4]] */

/*s: function [[prlost6]] */
static void
prlost6(ushort seq, void *v)
{
    Ip6hdr *ip6 = v;

    print("lost %ud: %I -> %I\n", seq, ip6->src, ip6->dst);
}
/*e: function [[prlost6]] */

/*s: function [[prreply4]] */
static void
prreply4(Req *r, void *v)
{
    Ip4hdr *ip4 = v;

    print("%ud: %V -> %V rtt %lld µs, avg rtt %lld µs, ttl = %d\n",
        r->seq - firstseq, ip4->src, ip4->dst, r->rtt, sum/rcvdmsgs,
        r->ttl);
}
/*e: function [[prreply4]] */

/*s: function [[prreply6]] */
static void
prreply6(Req *r, void *v)
{
    Ip6hdr *ip6 = v;

    print("%ud: %I -> %I rtt %lld µs, avg rtt %lld µs, ttl = %d\n",
        r->seq - firstseq, ip6->src, ip6->dst, r->rtt, sum/rcvdmsgs,
        r->ttl);
}
/*e: function [[prreply6]] */

/*s: global [[v4pr]] */
static Proto v4pr = {
    4,		"icmp",
    EchoRequest,	EchoReply,
    IPV4HDR_LEN,
    prreply4,	prlost4,
};
/*e: global [[v4pr]] */
/*s: global [[v6pr]] */
static Proto v6pr = {
    6,		"icmpv6",
    EchoRequestV6,	EchoReplyV6,
    IPV6HDR_LEN,
    prreply6,	prlost6,
};
/*e: global [[v6pr]] */

/*s: global [[proto]] */
static Proto *proto = &v4pr;
/*e: global [[proto]] */


/*s: function [[geticmp]] */
Icmphdr *
geticmp(void *v)
{
    char *p = v;

    return (Icmphdr *)(p + proto->iphdrsz);
}
/*e: function [[geticmp]] */

/*s: function [[clean]] */
void
clean(ushort seq, vlong now, void *v)
{
    int ttl;
    Req **l, *r;

    ttl = 0;
    if (v) {
        if (proto->version == 4)
            ttl = ((Ip4hdr *)v)->ttl;
        else
            ttl = ((Ip6hdr *)v)->ttl;
    }
    lock(&listlock);
    last = nil;
    for(l = &first; *l; ){
        r = *l;

        if(v && r->seq == seq){
            r->rtt = now-r->time;
            r->ttl = ttl;
            reply(r, v);
        }

        if(now-r->time > MINUTE){
            *l = r->next;
            r->rtt = now-r->time;
            if(v)
                r->ttl = ttl;
            if(r->replied == 0)
                lost(r, v);
            free(r);
        }else{
            last = r;
            l = &r->next;
        }
    }
    unlock(&listlock);
}
/*e: function [[clean]] */

/*s: global [[loopbacknet]]([[(networking/ip/ping.c)]]) */
static uchar loopbacknet[IPaddrlen] = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0xff, 0xff,
    127, 0, 0, 0
};
/*e: global [[loopbacknet]]([[(networking/ip/ping.c)]]) */
/*s: global [[loopbackmask]]([[(networking/ip/ping.c)]]) */
static uchar loopbackmask[IPaddrlen] = {
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,
    0xff, 0, 0, 0
};
/*e: global [[loopbackmask]]([[(networking/ip/ping.c)]]) */

/*s: function [[myipvnaddr]] */
/*
 * find first ip addr suitable for proto and
 * that isn't the friggin loopback address.
 * deprecate link-local and multicast addresses.
 */
static int
myipvnaddr(uchar *ip, Proto *proto, char *net)
{
    int ipisv4, wantv4;
    Ipifc *nifc;
    Iplifc *lifc;
    uchar mynet[IPaddrlen], linklocal[IPaddrlen];
    static Ipifc *ifc;

    ipmove(linklocal, IPnoaddr);
    wantv4 = proto->version == 4;
    ifc = readipifc(net, ifc, -1);
    for(nifc = ifc; nifc; nifc = nifc->next)
        for(lifc = nifc->lifc; lifc; lifc = lifc->next){
            maskip(lifc->ip, loopbackmask, mynet);
            if(ipcmp(mynet, loopbacknet) == 0)
                continue;
            if(ISIPV6MCAST(lifc->ip) || ISIPV6LINKLOCAL(lifc->ip)) {
                ipmove(linklocal, lifc->ip);
                continue;
            }
            ipisv4 = isv4(lifc->ip) != 0;
            if(ipcmp(lifc->ip, IPnoaddr) != 0 && wantv4 == ipisv4){
                ipmove(ip, lifc->ip);
                return 0;
            }
        }
    /* no global unicast addrs found, fall back to link-local, if any */
    ipmove(ip, linklocal);
    return ipcmp(ip, IPnoaddr) == 0? -1: 0;
}
/*e: function [[myipvnaddr]] */

/*s: function [[sender]] */
void
sender(int fd, int msglen, int interval, int n)
{
    int i, extra;
    ushort seq;
    char buf[64*1024+512];
    uchar me[IPaddrlen], mev4[IPv4addrlen];
    Icmphdr *icmp;
    Req *r;

    srand(time(0));
    firstseq = seq = rand();

    icmp = geticmp(buf);
    memset(buf, 0, proto->iphdrsz + ICMP_HDRSIZE);
    for(i = proto->iphdrsz + ICMP_HDRSIZE; i < msglen; i++)
        buf[i] = i;
    icmp->type = proto->echocmd;
    icmp->code = 0;

    /* arguably the kernel should fill in the right src addr. */
    myipvnaddr(me, proto, network);
    if (proto->version == 4) {
        v6tov4(mev4, me);
        memmove(((Ip4hdr *)buf)->src, mev4, IPv4addrlen);
    } else
        ipmove(((Ip6hdr *)buf)->src, me);
    if (addresses)
        print("\t%I -> %s\n", me, target);

    if(rint != 0 && interval <= 0)
        rint = 0;
    extra = 0;
    for(i = 0; i < n; i++){
        if(i != 0){
            if(rint != 0)
                extra = nrand(interval);
            sleep(interval + extra);
        }
        r = malloc(sizeof *r);
        if (r == nil)
            continue;
        hnputs(icmp->seq, seq);
        r->seq = seq;
        r->next = nil;
        r->replied = 0;
        r->time = nsec();	/* avoid early free in reply! */
        lock(&listlock);
        if(first == nil)
            first = r;
        else
            last->next = r;
        last = r;
        unlock(&listlock);
        r->time = nsec();
        if(write(fd, buf, msglen) < msglen){
            fprint(2, "%s: write failed: %r\n", argv0);
            return;
        }
        seq++;
    }
    done = 1;
}
/*e: function [[sender]] */

/*s: function [[rcvr]] */
void
rcvr(int fd, int msglen, int interval, int nmsg)
{
    int i, n, munged;
    ushort x;
    vlong now;
    uchar buf[64*1024+512];
    Icmphdr *icmp;
    Req *r;

    sum = 0;
    while(lostmsgs+rcvdmsgs < nmsg){
        alarm((nmsg-lostmsgs-rcvdmsgs)*interval+waittime);
        n = read(fd, buf, sizeof buf);
        alarm(0);
        now = nsec();
        if(n <= 0){	/* read interrupted - time to go */
            clean(0, now+MINUTE, nil);
            continue;
        }
        if(n < msglen){
            print("bad len %d/%d\n", n, msglen);
            continue;
        }
        icmp = geticmp(buf);
        munged = 0;
        for(i = proto->iphdrsz + ICMP_HDRSIZE; i < msglen; i++)
            if(buf[i] != (uchar)i)
                munged++;
        if(munged)
            print("corrupted reply\n");
        x = nhgets(icmp->seq);
        if(icmp->type != proto->echoreply || icmp->code != 0) {
            print("bad type/code/sequence %d/%d/%d (want %d/%d/%d)\n",
                icmp->type, icmp->code, x,
                proto->echoreply, 0, x);
            continue;
        }
        clean(x, now, buf);
    }

    lock(&listlock);
    for(r = first; r; r = r->next)
        if(r->replied == 0)
            lostmsgs++;
    unlock(&listlock);

    if(!quiet && lostmsgs)
        print("%d out of %d messages lost\n", lostmsgs,
            lostmsgs+rcvdmsgs);
}
/*e: function [[rcvr]] */

/*s: function [[isdottedquad]] */
static int
isdottedquad(char *name)
{
    int dot = 0, digit = 0;

    for (; *name != '\0'; name++)
        if (*name == '.')
            dot++;
        else if (isdigit(*name))
            digit++;
        else
            return 0;
    return dot && digit;
}
/*e: function [[isdottedquad]] */

/*s: function [[isv6lit]] */
static int
isv6lit(char *name)
{
    int colon = 0, hex = 0;

    for (; *name != '\0'; name++)
        if (*name == ':')
            colon++;
        else if (isxdigit(*name))
            hex++;
        else
            return 0;
    return colon;
}
/*e: function [[isv6lit]] */

/*s: enum [[_anon_ (networking/ip/ping.c)2]] */
/* from /sys/src/libc/9sys/dial.c */

enum
{
    Maxstring	= 128,
    Maxpath		= 256,
};
/*e: enum [[_anon_ (networking/ip/ping.c)2]] */

typedef struct DS DS;
/*s: struct [[DS]] */
struct DS {
    /* dist string */
    char	buf[Maxstring];
    char	*netdir;
    char	*proto;
    char	*rem;

    /* other args */
    char	*local;
    char	*dir;
    int	*cfdp;
};
/*e: struct [[DS]] */

/*s: function [[_dial_string_parse]] */
/*
 *  parse a dial string
 */
static void
_dial_string_parse(char *str, DS *ds)
{
    char *p, *p2;

    strncpy(ds->buf, str, Maxstring);
    ds->buf[Maxstring-1] = 0;

    p = strchr(ds->buf, '!');
    if(p == 0) {
        ds->netdir = 0;
        ds->proto = "net";
        ds->rem = ds->buf;
    } else {
        if(*ds->buf != '/' && *ds->buf != '#'){
            ds->netdir = 0;
            ds->proto = ds->buf;
        } else {
            for(p2 = p; *p2 != '/'; p2--)
                ;
            *p2++ = 0;
            ds->netdir = ds->buf;
            ds->proto = p2;
        }
        *p = 0;
        ds->rem = p + 1;
    }
}
/*e: function [[_dial_string_parse]] */

/* end excerpt from /sys/src/libc/9sys/dial.c */

/*s: function [[isv4name]] */
/* side effect: sets network & target */
static int
isv4name(char *name)
{
    int r = 1;
    char *root, *ip, *pr;
    DS ds;

    _dial_string_parse(name, &ds);

    /* cope with leading /net.alt/icmp! and the like */
    root = nil;
    if (ds.netdir != nil) {
        pr = strrchr(ds.netdir, '/');
        if (pr == nil)
            pr = ds.netdir;
        else {
            *pr++ = '\0';
            root = ds.netdir;
            network = strdup(root);
        }
        if (strcmp(pr, v4pr.net) == 0)
            return 1;
        if (strcmp(pr, v6pr.net) == 0)
            return 0;
    }

    /* if it's a literal, it's obvious from syntax which proto it is */
    free(target);
    target = strdup(ds.rem);
    if (isdottedquad(ds.rem))
        return 1;
    else if (isv6lit(ds.rem))
        return 0;

    /* map name to ip and look at its syntax */
    ip = csgetvalue(root, "sys", ds.rem, "ip", nil);
    if (ip == nil)
        ip = csgetvalue(root, "dom", ds.rem, "ip", nil);
    if (ip == nil)
        ip = csgetvalue(root, "sys", ds.rem, "ipv6", nil);
    if (ip == nil)
        ip = csgetvalue(root, "dom", ds.rem, "ipv6", nil);
    if (ip != nil)
        r = isv4name(ip);
    free(ip);
    return r;
}
/*e: function [[isv4name]] */

/*s: function [[main]]([[(networking/ip/ping.c)]]) */
void
main(int argc, char **argv)
{
    int fd, msglen, interval, nmsg;
    char *ds;

    nsec();		/* make sure time file is already open */

    fmtinstall('V', eipfmt);
    fmtinstall('I', eipfmt);

    msglen = interval = 0;
    nmsg = MAXMSG;
    ARGBEGIN {
    case '6':
        proto = &v6pr;
        break;
    case 'a':
        addresses = 1;
        break;
    case 'd':
        debug++;
        break;
    case 'f':
        flood = 1;
        break;
    case 'i':
        interval = atoi(EARGF(usage()));
        if(interval < 0)
            usage();
        break;
    case 'l':
        lostonly++;
        break;
    case 'n':
        nmsg = atoi(EARGF(usage()));
        if(nmsg < 0)
            usage();
        break;
    case 'q':
        quiet = 1;
        break;
    case 'r':
        rint = 1;
        break;
    case 's':
        msglen = atoi(EARGF(usage()));
        break;
    case 'w':
        waittime = atoi(EARGF(usage()));
        if(waittime < 0)
            usage();
        break;
    default:
        usage();
        break;
    } ARGEND;

    if(msglen < proto->iphdrsz + ICMP_HDRSIZE)
        msglen = proto->iphdrsz + ICMP_HDRSIZE;
    if(msglen < 64)
        msglen = 64;
    if(msglen >= 64*1024)
        msglen = 64*1024-1;
    if(interval <= 0 && !flood)
        interval = SLEEPMS;

    if(argc < 1)
        usage();

    notify(catch);

    if (!isv4name(argv[0]))
        proto = &v6pr;
    ds = netmkaddr(argv[0], proto->net, "1");
    fd = dial(ds, 0, 0, 0);
    if(fd < 0){
        fprint(2, "%s: couldn't dial %s: %r\n", argv0, ds);
        exits("dialing");
    }

    if (!quiet)
        print("sending %d %d byte messages %d ms apart to %s\n",
            nmsg, msglen, interval, ds);

    switch(rfork(RFPROC|RFMEM|RFFDG)){
    case -1:
        fprint(2, "%s: can't fork: %r\n", argv0);
        /* fallthrough */
    case 0:
        rcvr(fd, msglen, interval, nmsg);
        exits(0);
    default:
        sender(fd, msglen, interval, nmsg);
        wait();
        exits(lostmsgs ? "lost messages" : "");
    }
}
/*e: function [[main]]([[(networking/ip/ping.c)]]) */

/*s: function [[reply]] */
void
reply(Req *r, void *v)
{
    r->rtt /= 1000LL;
    sum += r->rtt;
    if(!r->replied)
        rcvdmsgs++;
    if(!quiet && !lostonly)
        if(addresses)
            (*proto->prreply)(r, v);
        else
            print("%ud: rtt %lld µs, avg rtt %lld µs, ttl = %d\n",
                r->seq - firstseq, r->rtt, sum/rcvdmsgs, r->ttl);
    r->replied = 1;
}
/*e: function [[reply]] */

/*s: function [[lost]] */
void
lost(Req *r, void *v)
{
    if(!quiet)
        if(addresses && v != nil)
            (*proto->prlost)(r->seq - firstseq, v);
        else
            print("lost %ud\n", r->seq - firstseq);
    lostmsgs++;
}
/*e: function [[lost]] */
/*e: networking/ip/ping.c */
