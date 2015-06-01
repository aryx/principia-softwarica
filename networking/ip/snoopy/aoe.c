/*s: networking/ip/snoopy/aoe.c */
#include <u.h>
#include <libc.h>
#include <ip.h>
#include "dat.h"
#include "protos.h"

typedef struct{
    uchar	verflags;
    uchar	error;
    uchar	major[2];
    uchar	minor;
    uchar	cmd;
    uchar	tag[4];
}Hdr;

/*s: enum _anon_ (networking/ip/snoopy/aoe.c) */
enum{
    Hsize	= 10,
};
/*e: enum _anon_ (networking/ip/snoopy/aoe.c) */

/*s: enum _anon_ (networking/ip/snoopy/aoe.c)2 */
enum{
    Omajor,
    Ominor,
    Ocmd,
};
/*e: enum _anon_ (networking/ip/snoopy/aoe.c)2 */

/*s: global p_mux (networking/ip/snoopy/aoe.c) */
static Mux p_mux[] = {
    {"aoeata",	0},
    {"aoecmd",	1},
    {"aoemask",	2},
    {"aoerr",	3},
    {0},
};
/*e: global p_mux (networking/ip/snoopy/aoe.c) */

/*s: global p_fields (networking/ip/snoopy/aoe.c) */
static Field p_fields[] =
{
    {"shelf",	Fnum,	Ominor,		"shelf", },
    {"slot",	Fnum,	Omajor,		"slot",	},
    {"cmd",		Fnum,	Ocmd,		"cmd",	},
    {0}
};
/*e: global p_fields (networking/ip/snoopy/aoe.c) */

/*s: function p_compile (networking/ip/snoopy/aoe.c) */
static void
p_compile(Filter *f)
{
    Mux *m;

    if(f->op == '='){
        compile_cmp(aoe.name, f, p_fields);
        return;
    }
    for(m = p_mux; m->name; m++)
        if(strcmp(f->s, m->name) == 0){
            f->pr = m->pr;
            f->ulv = m->val;
            f->subop = Ocmd;
            return;
        }
    sysfatal("unknown aoe field: %s", f->s);
}
/*e: function p_compile (networking/ip/snoopy/aoe.c) */

/*s: function p_filter (networking/ip/snoopy/aoe.c) */
static int
p_filter(Filter *f, Msg *m)
{
    Hdr *h;

    if(m->pe - m->ps < Hsize)
        return 0;

    h = (Hdr*)m->ps;
    m->ps += Hsize;

    switch(f->subop){
    case Omajor:
        return NetS(h->major) == f->ulv;
    case Ominor:
        return h->minor == f->ulv;
    case Ocmd:
        return h->cmd == f->ulv;
    }
    return 0;
}
/*e: function p_filter (networking/ip/snoopy/aoe.c) */

/*s: function p_seprint (networking/ip/snoopy/aoe.c) */
static int
p_seprint(Msg *m)
{
    Hdr *h;

    if(m->pe - m->ps < Hsize)
        return 0;

    h = (Hdr*)m->ps;
    m->ps += Hsize;

    demux(p_mux, h->cmd, h->cmd, m, &dump);

    m->p = seprint(m->p, m->e, "ver=%d flag=%4b err=%d %d.%d cmd=%ux tag=%ux",
        h->verflags >> 4, h->verflags & 0xf, h->error, NetS(h->major),
        h->minor, h->cmd, NetL(h->tag));
    return 0;
}
/*e: function p_seprint (networking/ip/snoopy/aoe.c) */

/*s: global aoe */
Proto aoe =
{
    "aoe",
    p_compile,
    p_filter,
    p_seprint,
    p_mux,
    "%lud",
    p_fields,
    defaultframer,
};
/*e: global aoe */
/*e: networking/ip/snoopy/aoe.c */
