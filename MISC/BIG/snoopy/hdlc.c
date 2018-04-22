/*s: networking/ip/snoopy/hdlc.c */
#include <u.h>
#include <libc.h>
#include <ip.h>
#include "dat.h"
#include "protos.h"

/*s: enum [[_anon_ (networking/ip/snoopy/hdlc.c)]] */
enum {
    HDLC_frame=	0x7e,
    HDLC_esc=	0x7d,

    /* PPP frame fields */
    PPP_addr=	0xff,
    PPP_ctl=	0x3,
    PPP_initfcs=	0xffff,
    PPP_goodfcs=	0xf0b8,
};
/*e: enum [[_anon_ (networking/ip/snoopy/hdlc.c)]] */

/*s: global [[fcstab]] */
/*
 * Calculate FCS - rfc 1331
 */
ushort fcstab[256] =
{
      0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
      0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
      0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
      0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
      0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
      0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
      0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
      0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
      0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
      0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
      0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
      0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
      0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
      0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
      0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
      0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
      0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
      0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
      0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
      0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
      0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
      0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
      0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
      0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
      0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
      0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
      0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
      0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
      0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
      0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
      0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
      0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};
/*e: global [[fcstab]] */

/*s: global [[inbuf]] */
static uchar inbuf[16*1024];
/*e: global [[inbuf]] */
/*s: global [[inlen]] */
static int inlen;
/*e: global [[inlen]] */

/*s: global [[p_mux]]([[(networking/ip/snoopy/hdlc.c)]]) */
static Mux p_mux[] =
{
    {"ppp",		(PPP_addr<<8)|PPP_ctl,	} ,
    {0}
};
/*e: global [[p_mux]]([[(networking/ip/snoopy/hdlc.c)]]) */

/*s: enum [[_anon_ (networking/ip/snoopy/hdlc.c)2]] */
enum
{
    Ot = 1
};
/*e: enum [[_anon_ (networking/ip/snoopy/hdlc.c)2]] */

/*s: function [[p_compile]]([[(networking/ip/snoopy/hdlc.c)]]) */
static void
p_compile(Filter *f)
{
    Mux *m;

    for(m = p_mux; m->name != nil; m++)
        if(strcmp(f->s, m->name) == 0){
            f->pr = m->pr;
            f->ulv = m->val;
            f->subop = Ot;
            return;
        }
    sysfatal("unknown ethernet field or protocol: %s", f->s);
}
/*e: function [[p_compile]]([[(networking/ip/snoopy/hdlc.c)]]) */

/*s: function [[p_filter]]([[(networking/ip/snoopy/hdlc.c)]]) */
static int
p_filter(Filter *f, Msg *m)
{
    ulong t;

    if(m->pe-m->ps < 2)
        return 0;

    switch(f->subop){
    case Ot:
        t = (m->ps[0]<<8)|m->ps[1];
        if(t != f->ulv)
            return 0;
        break;
    }
    return 1;
}
/*e: function [[p_filter]]([[(networking/ip/snoopy/hdlc.c)]]) */

/*s: function [[p_seprint]]([[(networking/ip/snoopy/hdlc.c)]]) */
static int
p_seprint(Msg *m)
{
    ulong t;

    if(m->pe-m->ps < 2)
        return -1;

    t = (m->ps[0]<<8)|m->ps[1];
    m->ps += 2;
    demux(p_mux, t, t, m, &dump);

    return 0;
}
/*e: function [[p_seprint]]([[(networking/ip/snoopy/hdlc.c)]]) */

/*s: function [[p_framer]] */
static int
p_framer(int fd, uchar *pkt, int pktlen)
{
    ushort fcs;
    uchar *from, *efrom, *to, *eto;
    int n;
    ulong c;

    eto = pkt+pktlen;
    for(;;){
        efrom = memchr(inbuf, HDLC_frame, inlen);
        if(efrom == nil){
            if(inlen >= sizeof(inbuf))
                inlen = 0;
            n = read(fd, inbuf+inlen, sizeof(inbuf)-inlen);
            if(n <= 0)
                break;
            inlen += n;
            continue;
        }

        /* checksum and unescape the frame */
        fcs = PPP_initfcs;
        to = pkt;
        for(from = inbuf; from < efrom;){
            c = *from++;
            if(c == HDLC_esc)
                c = (*from++) ^ 0x20;
            if(to < eto)
                *to++ = c;
            fcs = (fcs >> 8) ^ fcstab[(fcs ^ c) & 0xff];
        }

        /* move down anything that's left */
        inlen -= efrom+1-inbuf;
        memmove(inbuf, efrom+1, inlen);

        /* accept if this is a good packet */
        if(fcs != PPP_goodfcs)
            print("bad frame %ld %2.2ux %2.2ux!\n", to-pkt, pkt[0], pkt[1]);
        else
            return to-pkt;
    }
    return -1;
}
/*e: function [[p_framer]] */

/*s: global [[hdlc]] */
Proto hdlc =
{
    "hdlc",
    p_compile,
    p_filter,
    p_seprint,
    p_mux,
    "%#.4lux",
    nil,
    p_framer,
};
/*e: global [[hdlc]] */
/*e: networking/ip/snoopy/hdlc.c */