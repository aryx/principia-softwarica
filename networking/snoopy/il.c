/*s: networking/ip/snoopy/il.c */
#include <u.h>
#include <libc.h>
#include <ip.h>
#include "dat.h"
#include "protos.h"

typedef struct Hdr	Hdr;
/*s: struct [[Hdr]]([[(networking/ip/snoopy/il.c)]]) */
struct Hdr
{
    uchar	sum[2];		/* Checksum including header */
    uchar	len[2];		/* Packet length */
    uchar	type;		/* Packet type */
    uchar	spec;		/* Special */
    uchar	sport[2];	/* Src port */
    uchar	dport[2];	/* Dst port */
    uchar	id[4];		/* Sequence id */
    uchar	ack[4];		/* Acked sequence */
};
/*e: struct [[Hdr]]([[(networking/ip/snoopy/il.c)]]) */

/*s: enum [[_anon_ (networking/ip/snoopy/il.c)]] */
enum
{
    ILLEN= 18,
};
/*e: enum [[_anon_ (networking/ip/snoopy/il.c)]] */

/*s: enum [[_anon_ (networking/ip/snoopy/il.c)2]] */
enum
{
    Os,
    Od,
    Osd,
};
/*e: enum [[_anon_ (networking/ip/snoopy/il.c)2]] */

/*s: global [[p_fields]]([[(networking/ip/snoopy/il.c)]]) */
static Field p_fields[] = 
{
    {"s",		Fnum,	Os,	"source port",	} ,
    {"d",		Fnum,	Od,	"dest port",	} ,
    {"a",		Fnum,	Osd,	"source/dest port",	} ,
    {"sd",		Fnum,	Osd,	"source/dest port",	} ,
    {0}
};
/*e: global [[p_fields]]([[(networking/ip/snoopy/il.c)]]) */

/*s: global [[p_mux]]([[(networking/ip/snoopy/il.c)]]) */
static Mux p_mux[] =
{
    {"ninep",	17007, },	/* exportfs */
    {"ninep",	17008, },	/* 9fs */
    {"ninep",	17005, },	/* ocpu */
    {"ninep",	17010, },	/* ncpu */
    {"ninep",	17013, },	/* cpu */
    {0},
};
/*e: global [[p_mux]]([[(networking/ip/snoopy/il.c)]]) */

/*s: function [[p_compile]]([[(networking/ip/snoopy/il.c)]]) */
static void
p_compile(Filter *f)
{
    Mux *m;

    if(f->op == '='){
        compile_cmp(il.name, f, p_fields);
        return;
    }
    for(m = p_mux; m->name != nil; m++)
        if(strcmp(f->s, m->name) == 0){
            f->pr = m->pr;
            f->ulv = m->val;
            f->subop = Osd;
            return;
        }
    sysfatal("unknown il field or protocol: %s", f->s);
}
/*e: function [[p_compile]]([[(networking/ip/snoopy/il.c)]]) */

/*s: function [[p_filter]]([[(networking/ip/snoopy/il.c)]]) */
static int
p_filter(Filter *f, Msg *m)
{
    Hdr *h;

    if(m->pe - m->ps < ILLEN)
        return 0;
    h = (Hdr*)m->ps;
    m->ps += ILLEN;

    switch(f->subop){
    case Os:
        return NetS(h->sport) == f->ulv;
    case Od:
        return NetS(h->dport) == f->ulv;
    case Osd:
        return NetS(h->sport) == f->ulv || NetS(h->dport) == f->ulv;
    }
    return 0;
}
/*e: function [[p_filter]]([[(networking/ip/snoopy/il.c)]]) */

/*s: global [[pktnames]] */
char *pktnames[] = 
{
    "Sync",	
    "Data",
    "Dataquery",
    "Ack",
    "Query",
    "State",
    "Close"
};
/*e: global [[pktnames]] */

/*s: function [[pkttype]] */
static char*
pkttype(int t)
{
    static char b[10];
    
    if(t > 6){
        sprint(b, "%d", t);
        return b;
    }
    return pktnames[t];
}
/*e: function [[pkttype]] */

/*s: function [[p_seprint]]([[(networking/ip/snoopy/il.c)]]) */
static int
p_seprint(Msg *m)
{
    Hdr *h;
    int dport, sport;

    if(m->pe - m->ps < ILLEN)
        return -1;
    h = (Hdr*)m->ps;
    m->ps += ILLEN;

    dport = NetS(h->dport);
    sport = NetS(h->sport);
    demux(p_mux, sport, dport, m, &dump);

    m->p = seprint(m->p, m->e, "s=%d d=%d t=%s id=%lud ack=%lud spec=%d ck=%4.4ux ln=%d",
            sport, dport, pkttype(h->type),
            (ulong)NetL(h->id), (ulong)NetL(h->ack),
            h->spec,
            NetS(h->sum), NetS(h->len));
    return 0;
}
/*e: function [[p_seprint]]([[(networking/ip/snoopy/il.c)]]) */

/*s: global [[il]] */
Proto il =
{
    "il",
    p_compile,
    p_filter,
    p_seprint,
    p_mux,
    "%lud",
    p_fields,
    defaultframer,
};
/*e: global [[il]] */
/*e: networking/ip/snoopy/il.c */
