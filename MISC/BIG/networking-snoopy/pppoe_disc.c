/*s: networking/ip/snoopy/pppoe_disc.c */
#include <u.h>
#include <libc.h>
#include <ip.h>
#include "dat.h"
#include "protos.h"

typedef struct Hdr Hdr;
/*s: struct Hdr (networking/ip/snoopy/pppoe_disc.c) */
struct Hdr {
    uchar verstype;
    uchar code;
    uchar sessid[2];
    uchar length[2];	/* of payload */
};
/*e: struct Hdr (networking/ip/snoopy/pppoe_disc.c) */
/*s: enum _anon_ (networking/ip/snoopy/pppoe_disc.c) */
enum
{
    HDRSIZE = 1+1+2+2
};
/*e: enum _anon_ (networking/ip/snoopy/pppoe_disc.c) */

/*s: global p_mux (networking/ip/snoopy/pppoe_disc.c) */
static Mux p_mux[] =
{
    {"ppp",		0,	} ,
    {0}
};
/*e: global p_mux (networking/ip/snoopy/pppoe_disc.c) */

/*s: enum _anon_ (networking/ip/snoopy/pppoe_disc.c)2 */
enum
{
    Overs,
    Otype,
    Ocode,
    Osess,
};
/*e: enum _anon_ (networking/ip/snoopy/pppoe_disc.c)2 */

/*s: global p_fields (networking/ip/snoopy/pppoe_disc.c) */
static Field p_fields[] = 
{
    {"v",	Fnum,	Overs,	"version",	} ,
    {"t",	Fnum,	Otype,	"type",	} ,
    {"c",	Fnum,	Ocode,	"code" } ,
    {"s",	Fnum,	Osess,	"sessid" } ,
    {0}
};
/*e: global p_fields (networking/ip/snoopy/pppoe_disc.c) */

/*s: function p_compilesess */
static void
p_compilesess(Filter *f)
{
//	Mux *m;

    if(f->op == '='){
        compile_cmp(pppoe_sess.name, f, p_fields);
        return;
    }
/*
    for(m = p_mux; m->name != nil; m++)
        if(strcmp(f->s, m->name) == 0){
            f->pr = m->pr;
            f->ulv = m->val;
            f->subop = Ot;
            return;
        }
*/
    sysfatal("unknown pppoe field or protocol: %s", f->s);
}
/*e: function p_compilesess */
/*s: function p_compiledisc */
static void
p_compiledisc(Filter *f)
{
//	Mux *m;

    if(f->op == '='){
        compile_cmp(pppoe_disc.name, f, p_fields);
        return;
    }
/*
    for(m = p_mux; m->name != nil; m++)
        if(strcmp(f->s, m->name) == 0){
            f->pr = m->pr;
            f->ulv = m->val;
            f->subop = Ot;
            return;
        }
*/
    sysfatal("unknown pppoe field or protocol: %s", f->s);
}
/*e: function p_compiledisc */

/*s: function p_filter (networking/ip/snoopy/pppoe_disc.c) */
static int
p_filter(Filter *f, Msg *m)
{
    Hdr *h;

    if(m->pe - m->ps < HDRSIZE)
        return 0;

    h = (Hdr*)m->ps;
    m->ps += HDRSIZE;

    switch(f->subop){
    case Overs:
        return (h->verstype>>4) == f->ulv;
    case Otype:
        return (h->verstype&0xF) == f->ulv;
    case Ocode:
        return h->code == f->ulv;
    case Osess:
        return NetS(h->sessid) == f->ulv;
    }
    return 0;
}
/*e: function p_filter (networking/ip/snoopy/pppoe_disc.c) */

/*s: function p_seprintdisc */
/* BUG: print all the discovery types */
static int
p_seprintdisc(Msg *m)
{
    Hdr *h;
    int len;

    len = m->pe - m->ps;
    if(len < HDRSIZE)
        return -1;

    h = (Hdr*)m->ps;
    m->ps += HDRSIZE;

    m->pr = nil;

    m->p = seprint(m->p, m->e, "v=%d t=%d c=0x%x s=0x%ux, len=%d",
        h->verstype>>4, h->verstype&0xF, h->code, NetS(h->sessid), NetS(h->length));

    return 0;
}
/*e: function p_seprintdisc */

/*s: function p_seprintsess */
static int
p_seprintsess(Msg *m)
{
    Hdr *h;
    int len;

    len = m->pe - m->ps;
    if(len < HDRSIZE)
        return -1;

    h = (Hdr*)m->ps;
    m->ps += HDRSIZE;

    /* this will call ppp for me */
    demux(p_mux, 0, 0, m, &dump);

    m->p = seprint(m->p, m->e, "v=%d t=%d c=0x%x s=0x%ux, len=%d",
        h->verstype>>4, h->verstype&0xF, h->code, NetS(h->sessid), NetS(h->length));

    return 0;
}
/*e: function p_seprintsess */

/*s: global pppoe_disc */
Proto pppoe_disc =
{
    "pppoe_disc",
    p_compiledisc,
    p_filter,
    p_seprintdisc,
    p_mux,
    "%lud",
    p_fields,
    defaultframer
};
/*e: global pppoe_disc */

/*s: global pppoe_sess */
Proto pppoe_sess =
{
    "pppoe_sess",
    p_compilesess,
    p_filter,
    p_seprintsess,
    p_mux,
    "%lud",
    p_fields,
    defaultframer
};
/*e: global pppoe_sess */

/*e: networking/ip/snoopy/pppoe_disc.c */
