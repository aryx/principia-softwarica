/*s: networking/ip/snoopy/ttls.c */
#include <u.h>
#include <libc.h>
#include <ip.h>
#include "dat.h"
#include "protos.h"

typedef struct Hdr	Hdr;
/*s: struct Hdr (networking/ip/snoopy/ttls.c) */
struct Hdr
{
    uchar	flags;
    uchar	ln[4];	/* optional, present if L flag set*/
};
/*e: struct Hdr (networking/ip/snoopy/ttls.c) */

/*s: enum _anon_ (networking/ip/snoopy/ttls.c) */
enum
{
    FLHDR=	1,	/* sizeof(flags) */
    LNHDR=	4,	/* sizeof(ln) */
};
/*e: enum _anon_ (networking/ip/snoopy/ttls.c) */

/*s: enum _anon_ (networking/ip/snoopy/ttls.c)2 */
enum
{
    FlagL = 1<<7, 
    FlagM = 1<<6,
    FlagS = 1<<5,
    Version = (1<<2)|(1<<1)|(1<<0),
};
/*e: enum _anon_ (networking/ip/snoopy/ttls.c)2 */

/*s: global p_mux (networking/ip/snoopy/ttls.c) */
static Mux p_mux[] =
{
    { "dump", 0, },
    { 0 }
};
/*e: global p_mux (networking/ip/snoopy/ttls.c) */

/*s: function flags (networking/ip/snoopy/ttls.c) */
static char*
flags(int f)
{
    static char fl[20];
    char *p;

    p = fl;
    if(f & FlagS)
        *p++ = 'S';
    if(f & FlagM)
        *p++ = 'M';
    if(f & FlagL)
        *p++ = 'L';
    *p = 0;
    return fl;
}
/*e: function flags (networking/ip/snoopy/ttls.c) */

/*s: function p_seprint (networking/ip/snoopy/ttls.c) */
static int
p_seprint(Msg *m)
{
    Hdr *h;

    if(m->pe - m->ps < FLHDR)
        return -1;

    h = (Hdr*)m->ps;
    m->ps += FLHDR;

    if (h->flags & FlagL) {
        if(m->pe - m->ps < LNHDR)
            return -1;
        else
            m->ps += LNHDR;
    }

    /* next protocol  depending on type*/
    demux(p_mux, 0, 0, m, &dump);

    m->p = seprint(m->p, m->e, "ver=%1d", h->flags & Version);
    m->p = seprint(m->p, m->e, " fl=%s", flags(h->flags));

    if (h->flags & FlagL)
        m->p = seprint(m->p, m->e, " totallen=%1d", NetL(h->ln));

    /* these are not in the header, just print them for our convenience */
    m->p = seprint(m->p, m->e, " dataln=%1ld", m->pe - m->ps);
    if ((h->flags & (FlagL|FlagS|FlagM)) == 0 && m->ps == m->pe)
        m->p = seprint(m->p, m->e, " ack");

    return 0;
}
/*e: function p_seprint (networking/ip/snoopy/ttls.c) */

/*s: global ttls */
Proto ttls =
{
    "ttls",
    nil,
    nil,
    p_seprint,
    p_mux, /* we need this to get the dump printed */
    "%lud",
    nil,
    defaultframer,
};
/*e: global ttls */
/*e: networking/ip/snoopy/ttls.c */
