/*s: networking/ip/snoopy/cec.c */
#include <u.h>
#include <libc.h>
#include <ip.h>
#include "dat.h"
#include "protos.h"

typedef struct{
    uchar	type;
    uchar	conn;
    uchar	seq;
    uchar	len;
}Hdr;

/*s: enum [[_anon_ (networking/ip/snoopy/cec.c)]] */
enum{
    Hsize	= 4,
};
/*e: enum [[_anon_ (networking/ip/snoopy/cec.c)]] */

/*s: enum [[_anon_ (networking/ip/snoopy/cec.c)2]] */
enum{
    Otype,
    Oconn,
    Oseq,
    Olen,
};
/*e: enum [[_anon_ (networking/ip/snoopy/cec.c)2]] */

/*s: global [[p_fields]]([[(networking/ip/snoopy/cec.c)]]) */
static Field p_fields[] =
{
    {"type",	Fnum,	Otype,		"type",	},
    {"conn",	Fnum,	Oconn,		"conn",	},
    {"seq",		Fnum,	Oseq,		"seq",	},
    {"len",		Fnum,	Olen,		"len",	},
    {0}
};
/*e: global [[p_fields]]([[(networking/ip/snoopy/cec.c)]]) */

/*s: function [[p_compile]]([[(networking/ip/snoopy/cec.c)]]) */
static void
p_compile(Filter *f)
{
    if(f->op == '='){
        compile_cmp(aoe.name, f, p_fields);
        return;
    }
    sysfatal("unknown aoe field: %s", f->s);
}
/*e: function [[p_compile]]([[(networking/ip/snoopy/cec.c)]]) */

/*s: function [[p_filter]]([[(networking/ip/snoopy/cec.c)]]) */
static int
p_filter(Filter *f, Msg *m)
{
    Hdr *h;

    if(m->pe - m->ps < Hsize)
        return 0;

    h = (Hdr*)m->ps;
    m->ps += Hsize;

    switch(f->subop){
    case Otype:
        return h->type == f->ulv;
    case Oconn:
        return h->conn = f->ulv;
    case Oseq:
        return h->seq = f->ulv;
    case Olen:
        return h->len = f->ulv;
    }
    return 0;
}
/*e: function [[p_filter]]([[(networking/ip/snoopy/cec.c)]]) */

/*s: global [[ttab]] */
static char* ttab[] = {
    "Tinita",
    "Tinitb",
    "Tinitc",
    "Tdata",
    "Tack",
    "Tdiscover",
    "Toffer",
    "Treset",
};
/*e: global [[ttab]] */

/*s: function [[p_seprint]]([[(networking/ip/snoopy/cec.c)]]) */
static int
p_seprint(Msg *m)
{
    char *s, *p, buf[4];
    Hdr *h;

    if(m->pe - m->ps < Hsize)
        return 0;

    h = (Hdr*)m->ps;
    m->ps += Hsize;

    m->pr = nil;

    if(h->type < nelem(ttab))
        s = ttab[h->type];
    else{
        snprint(buf, sizeof buf, "%d", h->type);
        s = buf;
    }

    p = (char*)m->ps;
    m->p = seprint(m->p, m->e, "type=%s conn=%d seq=%d len=%d %.*s",
        s, h->conn, h->seq, h->len,
        (int)utfnlen(p, h->len), p);
    return 0;
}
/*e: function [[p_seprint]]([[(networking/ip/snoopy/cec.c)]]) */

/*s: global [[cec]] */
Proto cec =
{
    "cec",
    p_compile,
    p_filter,
    p_seprint,
    nil,
    nil,
    p_fields,
    defaultframer,
};
/*e: global [[cec]] */
/*e: networking/ip/snoopy/cec.c */
