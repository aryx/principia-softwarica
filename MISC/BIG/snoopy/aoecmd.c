/*s: networking/ip/snoopy/aoecmd.c */
#include <u.h>
#include <libc.h>
#include <ip.h>
#include "dat.h"
#include "protos.h"

typedef struct{
    uchar	bc[2];
    uchar	fw[2];
    uchar	sc;
    uchar	ccmd;
    uchar	len[2];
}Hdr;

/*s: enum [[_anon_ (networking/ip/snoopy/aoecmd.c)]] */
enum{
    Hsize	= 8,
};
/*e: enum [[_anon_ (networking/ip/snoopy/aoecmd.c)]] */

/*s: enum [[_anon_ (networking/ip/snoopy/aoecmd.c)2]] */
enum{
    Ocmd,
};
/*e: enum [[_anon_ (networking/ip/snoopy/aoecmd.c)2]] */

/*s: global [[p_fields]]([[(networking/ip/snoopy/aoecmd.c)]]) */
static Field p_fields[] =
{
    {"cmd",		Fnum,	Ocmd,		"cmd",	},
    {0}
};
/*e: global [[p_fields]]([[(networking/ip/snoopy/aoecmd.c)]]) */

/*s: function [[p_compile]]([[(networking/ip/snoopy/aoecmd.c)]]) */
static void
p_compile(Filter *f)
{
    if(f->op == '='){
        compile_cmp(aoecmd.name, f, p_fields);
        return;
    }
    sysfatal("unknown aoecmd field: %s", f->s);
}
/*e: function [[p_compile]]([[(networking/ip/snoopy/aoecmd.c)]]) */

/*s: function [[p_filter]]([[(networking/ip/snoopy/aoecmd.c)]]) */
static int
p_filter(Filter *f, Msg *m)
{
    Hdr *h;

    if(m->pe - m->ps < Hsize)
        return 0;

    h = (Hdr*)m->ps;
    m->ps += Hsize;

    switch(f->subop){
    case Ocmd:
        return (h->ccmd & 0xf) == f->ulv;
    }
    return 0;
}
/*e: function [[p_filter]]([[(networking/ip/snoopy/aoecmd.c)]]) */

/*s: function [[p_seprint]]([[(networking/ip/snoopy/aoecmd.c)]]) */
static int
p_seprint(Msg *m)
{
    Hdr *h;

    if(m->pe - m->ps < Hsize)
        return 0;

    h = (Hdr*)m->ps;
    m->ps += Hsize;

    /* no next protocol */
    m->pr = nil;

    m->p = seprint(m->p, m->e, "bc=%d fw=%.4x sc=%d ver=%d ccmd=%d len=%d cfg=",
        NetS(h->bc), NetS(h->fw), h->sc, h->ccmd >> 4, h->ccmd & 0xf,
        NetS(h->len));
    m->p = seprint(m->p, m->e, "%.*s", NetS(h->len), (char*)m->ps);
    return 0;
}
/*e: function [[p_seprint]]([[(networking/ip/snoopy/aoecmd.c)]]) */

/*s: global [[aoecmd]] */
Proto aoecmd =
{
    "aoecmd",
    p_compile,
    p_filter,
    p_seprint,
    nil,
    nil,
    p_fields,
    defaultframer,
};
/*e: global [[aoecmd]] */
/*e: networking/ip/snoopy/aoecmd.c */
