/*s: networking/ip/snoopy/eapol_key.c */
#include <u.h>
#include <libc.h>
#include <ip.h>
#include "dat.h"
#include "protos.h"

/*s: struct [[Hdr]]([[(networking/ip/snoopy/eapol_key.c)]]) */
typedef struct Hdr
{
    uchar	desc;
} Hdr;
/*e: struct [[Hdr]]([[(networking/ip/snoopy/eapol_key.c)]]) */

/*s: struct [[Rc4KeyDesc]] */
typedef struct Rc4KeyDesc
{
    uchar	ln[2];
    uchar	replay[8];
    uchar	iv[16];
    uchar	idx;
    uchar	md[16];
} Rc4KeyDesc;
/*e: struct [[Rc4KeyDesc]] */

/*s: enum [[_anon_ (networking/ip/snoopy/eapol_key.c)]] */
enum
{
    HDR=	1,		/* sizeof(Hdr) */
    RC4KEYDESC=	43,	/* sizeof(Rc4KeyDesc) */

    DescTpRC4= 1,
};
/*e: enum [[_anon_ (networking/ip/snoopy/eapol_key.c)]] */

/*s: enum [[_anon_ (networking/ip/snoopy/eapol_key.c)2]] */
enum
{
    Odesc,
};
/*e: enum [[_anon_ (networking/ip/snoopy/eapol_key.c)2]] */

/*s: global [[p_mux]]([[(networking/ip/snoopy/eapol_key.c)]]) */
static Mux p_mux[] =
{
    { "rc4keydesc", DescTpRC4, },
    { 0 }
};
/*e: global [[p_mux]]([[(networking/ip/snoopy/eapol_key.c)]]) */

/*s: global [[p_muxrc4]] */
static Mux p_muxrc4[] =
{
    { "dump", 0, },
    { 0 }
};
/*e: global [[p_muxrc4]] */

/*s: function [[p_compile]]([[(networking/ip/snoopy/eapol_key.c)]]) */
static void
p_compile(Filter *f)
{
    Mux *m;

    for(m = p_mux; m->name != nil; m++)
        if(strcmp(f->s, m->name) == 0){
            f->pr = m->pr;
            f->ulv = m->val;
            f->subop = Odesc;
            return;
        }
    sysfatal("unknown eap_key field or type: %s", f->s);
}
/*e: function [[p_compile]]([[(networking/ip/snoopy/eapol_key.c)]]) */

/*s: function [[p_filter]]([[(networking/ip/snoopy/eapol_key.c)]]) */
static int
p_filter(Filter *f, Msg *m)
{
    Hdr *h;

    if(m->pe - m->ps < HDR)
        return 0;

    h = (Hdr*)m->ps;
    m->ps += HDR;

    switch(f->subop){
    case Odesc:
        return h->desc == f->ulv;
    }
    return 0;
}
/*e: function [[p_filter]]([[(networking/ip/snoopy/eapol_key.c)]]) */

/*s: function [[op]]([[(networking/ip/snoopy/eapol_key.c)]]) */
static char*
op(int i)
{
    static char x[20];

    switch(i){
    case DescTpRC4:
        return "RC4KeyDesc";
    default:
        sprint(x, "%1d", i);
        return x;
    }
}
/*e: function [[op]]([[(networking/ip/snoopy/eapol_key.c)]]) */

/*s: function [[p_seprint]]([[(networking/ip/snoopy/eapol_key.c)]]) */
static int
p_seprint(Msg *m)
{
    Hdr *h;

    if(m->pe - m->ps < HDR)
        return -1;

    h = (Hdr*)m->ps;
    m->ps += HDR;

    /* next protocol  depending on type*/
    demux(p_mux, h->desc, h->desc, m, &dump);

    m->p = seprint(m->p, m->e, "desc=%s", op(h->desc));
    return 0;
}
/*e: function [[p_seprint]]([[(networking/ip/snoopy/eapol_key.c)]]) */

/*s: function [[p_seprintrc4]] */
static int
p_seprintrc4(Msg *m)
{
    Rc4KeyDesc *h;
    int len;

    if(m->pe - m->ps < RC4KEYDESC)
        return -1;

    h = (Rc4KeyDesc*)m->ps;
    m->ps += RC4KEYDESC;
    m->pr = nil;
    len = m->pe - m->ps;

    m->p = seprint(m->p, m->e, "keylen=%1d replay=%1d iv=%1d idx=%1d md=%1d",
            NetS(h->ln), NetS(h->replay), NetS(h->iv), h->idx, NetS(h->md));
    m->p = seprint(m->p, m->e, " dataln=%d", len);
    if (len > 0)
        m->p = seprint(m->p, m->e, " data=%.*H", len, m->ps);
    return 0;
}
/*e: function [[p_seprintrc4]] */

/*s: global [[eapol_key]] */
Proto eapol_key =
{
    "eapol_key",
    p_compile,
    p_filter,
    p_seprint,
    p_mux,
    "%lud",
    nil,
    defaultframer,
};
/*e: global [[eapol_key]] */

/*s: global [[rc4keydesc]] */
Proto rc4keydesc =
{
    "rc4keydesc",
    p_compile,
    nil,
    p_seprintrc4,
    nil,
    nil,
    nil,
    defaultframer,
};
/*e: global [[rc4keydesc]] */
/*e: networking/ip/snoopy/eapol_key.c */
