/*s: networking/ip/snoopy/eapol.c */
#include <u.h>
#include <libc.h>
#include <ip.h>
#include "dat.h"
#include "protos.h"

typedef struct Hdr	Hdr;
/*s: struct [[Hdr]]([[(networking/ip/snoopy/eapol.c)]]) */
struct Hdr
{
    uchar	vi;		/* version */
    uchar	type;
    uchar	len[2];	/* length of data following this header */
};
/*e: struct [[Hdr]]([[(networking/ip/snoopy/eapol.c)]]) */

/*s: enum [[_anon_ (networking/ip/snoopy/eapol.c)]] */
enum
{
    EAPOLHDR=	4,		/* sizeof(Hdr) */

    /* eapol types */
    Eap = 0,
    Start,
    Logoff,
    Key,
    AsfAlert,
};
/*e: enum [[_anon_ (networking/ip/snoopy/eapol.c)]] */

/*s: enum [[_anon_ (networking/ip/snoopy/eapol.c)2]] */
enum
{
    Ot,	/* type */
};
/*e: enum [[_anon_ (networking/ip/snoopy/eapol.c)2]] */

/*s: global [[p_mux]]([[(networking/ip/snoopy/eapol.c)]]) */
static Mux p_mux[] =
{
    { "eap", Eap, },
    { "eapol_start", Start, },
    { "eapol_logoff", Logoff, },
    { "eapol_key", Key, },
    { "asf_alert", AsfAlert, },
    { 0 }
};
/*e: global [[p_mux]]([[(networking/ip/snoopy/eapol.c)]]) */

/*s: function [[p_compile]]([[(networking/ip/snoopy/eapol.c)]]) */
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
    sysfatal("unknown eapol field or type: %s", f->s);
}
/*e: function [[p_compile]]([[(networking/ip/snoopy/eapol.c)]]) */

/*s: function [[p_filter]]([[(networking/ip/snoopy/eapol.c)]]) */
static int
p_filter(Filter *f, Msg *m)
{
    Hdr *h;

    if(m->pe - m->ps < EAPOLHDR)
        return 0;

    h = (Hdr*)m->ps;

    /* len does not include header */
    m->ps += EAPOLHDR;

    switch(f->subop){
    case Ot:
        return h->type == f->ulv;
    }
    return 0;
}
/*e: function [[p_filter]]([[(networking/ip/snoopy/eapol.c)]]) */

/*s: function [[op]]([[(networking/ip/snoopy/eapol.c)]]) */
static char*
op(int i)
{
    static char x[20];

    switch(i){
    case Eap:
        return "Eap";
    case Start:
        return "Start";
    case Logoff:
        return "Logoff";
    case Key:
        return "Key";
    case AsfAlert:
        return "AsfAlert";
    default:
        sprint(x, "%1d", i);
        return x;
    }
}
/*e: function [[op]]([[(networking/ip/snoopy/eapol.c)]]) */

/*s: function [[p_seprint]]([[(networking/ip/snoopy/eapol.c)]]) */
static int
p_seprint(Msg *m)
{
    Hdr *h;
    int len;

    if(m->pe - m->ps < EAPOLHDR)
        return -1;

    h = (Hdr*)m->ps;

    /* len does not include header */
    m->ps += EAPOLHDR;

    /* truncate the message if there's extra */
    len = NetS(h->len);
    if(m->ps + len < m->pe)
        m->pe = m->ps + len;
    else if(m->ps+len > m->pe)
        return -1;

    /* next protocol  depending on type*/
    demux(p_mux, h->type, h->type, m, &dump);

    m->p = seprint(m->p, m->e, "type=%s version=%1d datalen=%1d",
            op(h->type), h->vi, len);
    return 0;
}
/*e: function [[p_seprint]]([[(networking/ip/snoopy/eapol.c)]]) */

/*s: global [[eapol]] */
Proto eapol =
{
    "eapol",
    p_compile,
    p_filter,
    p_seprint,
    p_mux,
    "%lud",
    nil,
    defaultframer,
};
/*e: global [[eapol]] */
/*e: networking/ip/snoopy/eapol.c */
