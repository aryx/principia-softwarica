/*s: networking/ip/measure.c */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <ip.h>

typedef struct Etherpkt	Etherpkt;
typedef struct Ippkt	Ippkt;

/*
 *  ether packet
 */
/*s: struct [[Etherpkt]]([[(networking/ip/measure.c)]]) */
struct Etherpkt {
    uchar d[6];
    uchar s[6];
    uchar type[2];
    char data[1500];
};
/*e: struct [[Etherpkt]]([[(networking/ip/measure.c)]]) */
/*s: constant [[ETHERMINTU]] */
#define	ETHERMINTU	60	/* minimum transmit size */
/*e: constant [[ETHERMINTU]] */
/*s: constant [[ETHERMAXTU]] */
#define	ETHERMAXTU	1514	/* maximum transmit size */
/*e: constant [[ETHERMAXTU]] */
/*s: constant [[ETHERHDRSIZE]] */
#define ETHERHDRSIZE	14	/* size of an ethernet header */
/*e: constant [[ETHERHDRSIZE]] */

/*
 *  ip packets
 */
/*s: struct [[Ippkt]] */
struct Ippkt
{
    uchar	vihl;		/* Version and header length */
    uchar	tos;		/* Type of service */
    uchar	length[2];	/* packet length */
    uchar	id[2];		/* Identification */
    uchar	frag[2];	/* Fragment information */
    uchar	ttl;		/* Time to live */
    uchar	proto;		/* Protocol */
    uchar	cksum[2];	/* Header checksum */
    uchar	src[4];		/* Ip source */
    uchar	dst[4];		/* Ip destination */
    char	data[1];
};
/*e: struct [[Ippkt]] */

/*s: constant [[IP_HDRSIZE]] */
#define IP_HDRSIZE	20
/*e: constant [[IP_HDRSIZE]] */
/*s: constant IP_UDPPROTO (measure.c) */
#define IP_UDPPROTO	17
/*e: constant IP_UDPPROTO (measure.c) */
/*s: constant [[IP_MBONEPROTO]] */
#define IP_MBONEPROTO	4
/*e: constant [[IP_MBONEPROTO]] */
/*s: constant [[IP_TCPPROTO]] */
#define IP_TCPPROTO	6
/*e: constant [[IP_TCPPROTO]] */
/*s: constant IP_ILPROTO (measure.c) */
#define	IP_ILPROTO	40
/*e: constant IP_ILPROTO (measure.c) */
/*s: constant [[IP_ICMPPROTO]] */
#define	IP_ICMPPROTO	1
/*e: constant [[IP_ICMPPROTO]] */
/*s: constant [[IP_DF]] */
#define	IP_DF		0x4000
/*e: constant [[IP_DF]] */
/*s: constant [[IP_MF]] */
#define	IP_MF		0x2000
/*e: constant [[IP_MF]] */

/*s: macro [[NetS]] */
#define NetS(x) (((x)[0]<<8) | (x)[1])
/*e: macro [[NetS]] */
/*s: macro [[NetL]] */
#define NetL(x) (((x)[0]<<24) | ((x)[1]<<16) | ((x)[2]<<8) | (x)[3])
/*e: macro [[NetL]] */

/*s: global [[debug]]([[(networking/ip/measure.c)]]) */
/*
 *  run flags
 */
int	debug;
/*e: global [[debug]]([[(networking/ip/measure.c)]]) */
/*s: global [[mbone]] */
int	mbone;
/*e: global [[mbone]] */

/*s: global [[protoin]] */
ulong protoin[256];
/*e: global [[protoin]] */
/*s: global [[protoout]] */
ulong protoout[256];
/*e: global [[protoout]] */
/*s: global [[protopin]] */
ulong protopin[256];
/*e: global [[protopin]] */
/*s: global [[protopout]] */
ulong protopout[256];
/*e: global [[protopout]] */

/*s: function [[error]] */
void
error(char *s)
{
    char buf[ERRMAX];

    errstr(buf, sizeof buf);
    fprint(2, "snoopy: %s %s\n", buf, s);
    exits("death");
}
/*e: function [[error]] */

/*s: function [[warning]]([[(networking/ip/measure.c)]]) */
void
warning(char *s)
{
    char buf[ERRMAX];

    errstr(buf, sizeof buf);
    fprint(2, "snoopy: %s %s\n", buf, s);
}
/*e: function [[warning]]([[(networking/ip/measure.c)]]) */

/*s: function [[printproto]] */
void
printproto(int p)
{
    print("\t%d(%ld %ld %ld %ld)", p, protoin[p], protopin[p], protoout[p], protopout[p]);
}
/*e: function [[printproto]] */

/*s: function [[main]]([[(networking/ip/measure.c)]]) */
void
main(int argc, char *argv[])
{
    Etherpkt e;
    Ippkt *ip;
    long n;
    int fd, cfd;
    int ts, len, t;
    long start;
    int delta;
    uchar target[6];
    char buf[256];
    ulong samples;

    samples = -1;
    ARGBEGIN{
    case 'd':
        debug++;
        break;
    case 's':
        samples = atoi(ARGF());
        break;
    }ARGEND;

    if(argc < 2){
        fprint(2, "usage: %s device ip-addr [minutes-per-sample]\n", argv0);
        exits("usage");
    }
    if(argc > 2)
        delta = atoi(argv[2])*60*1000;
    else
        delta = 5*60*1000;
    parseether(target, argv[1]);

    fmtinstall('E', eipfmt);
    fmtinstall('I', eipfmt);

    snprint(buf, sizeof(buf), "%s!-2", argv[0]);
    fd = dial(buf, 0, 0, &cfd);
    if(fd < 0)
        error("opening ether data");
    if(write(cfd, "promiscuous", sizeof("promiscuous")-1) <= 0)
        error("connecting");

    start = 0;
    fd = -1;

    for(;;){
        if(fd < 0){
            fd = dial(buf, 0, 0, &cfd);
            if(fd < 0)
                error("opening ether data");
            if(write(cfd, "promiscuous", sizeof("promiscuous")-1) <= 0)
                error("connecting");
            close(cfd);
        }
        n = read(fd, &e, sizeof(e));
        if(n <= 0)
            break;
        ts = NetL(&e.d[60]);
        n = NetS(&e.d[58]) - ETHERHDRSIZE;
        if(n < 0)
            continue;
        if(start == 0)
            start = ts;
        t = NetS(e.type);
        if(t == 0x0800 || (t&0xFF00) == 0x1000){
            ip = (Ippkt*)e.data;
            len = NetS(ip->length);
            if(len > n)
                len = n;
            if(debug)
                fprint(2, "%I -> %I %d\n", ip->src, ip->dst, len);
            if(memcmp(e.s, target, 6) == 0){
                protopin[0]++;
                protoin[0] += len;
                if(ip->proto){
                    protopin[ip->proto]++;
                    protoin[ip->proto] += len;
                }
            }
            if(memcmp(e.d, target, 6) == 0){
                protopout[0]++;
                protoout[0] += len;
                if(ip->proto){
                    protopout[ip->proto]++;
                    protoout[ip->proto] += len;
                }
            }
        }
        if(ts - start >= delta){
            print("%8.8ld %ld", time(0), ts - start);
            printproto(0);
            printproto(IP_MBONEPROTO);
            printproto(IP_UDPPROTO);
            printproto(IP_TCPPROTO);
            print("\n");
            start = 0;
            memset(protoin, 0, sizeof(protoin));
            memset(protoout, 0, sizeof(protoout));
            memset(protopin, 0, sizeof(protopin));
            memset(protopout, 0, sizeof(protopout));
            close(fd);
            fd = -1;
            if(--samples == 0)
                break;
        }
    }
    exits(0);
}
/*e: function [[main]]([[(networking/ip/measure.c)]]) */
/*e: networking/ip/measure.c */
