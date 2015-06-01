/*s: networking/ip/snoopy/aoemask.c */
#include <u.h>
#include <libc.h>
#include <ip.h>
#include "dat.h"
#include "protos.h"

typedef struct {
    uchar	res;
    uchar	cmd;
    uchar	err;
    uchar	cnt;
} Hdr;

/*s: enum _anon_ (networking/ip/snoopy/aoemask.c) */
enum {
    Ocmd,
    Oerr,
    Ocnt,

    Hsize	= 4,
};
/*e: enum _anon_ (networking/ip/snoopy/aoemask.c) */

/*s: global p_fields (networking/ip/snoopy/aoemask.c) */
static Field p_fields[] =
{
    { "cmd",	Fnum,	Ocmd,	"command", },
    { "err",	Fnum,	Oerr,	"error", },
    { "cnt",	Fnum,	Ocnt,	"count", },
    nil
};
/*e: global p_fields (networking/ip/snoopy/aoemask.c) */

/*s: global p_mux (networking/ip/snoopy/aoemask.c) */
static Mux p_mux[] = {
    { "aoemd",	0 },
    { "aoemd",	1 },
    nil
};
/*e: global p_mux (networking/ip/snoopy/aoemask.c) */

/*s: function p_compile (networking/ip/snoopy/aoemask.c) */
static void
p_compile(Filter *f)
{
    Mux *m;

    if(f->op == '='){
        compile_cmp(aoerr.name, f, p_fields);
        return;
    }
    for(m = p_mux; m->name; m++)
        if(strcmp(f->s, m->name) == 0){
            f->pr = m->pr;
            f->ulv = m->val;
            f->subop = Ocmd;
            return;
        }
    sysfatal("unknown aoemask field: %s", f->s);
}
/*e: function p_compile (networking/ip/snoopy/aoemask.c) */

/*s: function p_filter (networking/ip/snoopy/aoemask.c) */
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
        return h->cmd == f->ulv;
    case Oerr:
        return h->err == f->ulv;
    case Ocnt:
        return h->cnt == f->ulv;
    }
    return 0;
}
/*e: function p_filter (networking/ip/snoopy/aoemask.c) */

/*s: global ctab */
static char *ctab[] = {
    "read",
    "edit",
};
/*e: global ctab */

/*s: global etab */
static char *etab[] = {
    "",
    "bad",
    "full",
};
/*e: global etab */

/*s: function p_seprint (networking/ip/snoopy/aoemask.c) */
static int
p_seprint(Msg *m)
{
    char *s, *t;
    Hdr *h;

    if(m->pe - m->ps < Hsize)
        return 0;

    h = (Hdr*)m->ps;
    m->ps += Hsize;

    demux(p_mux, h->cmd, h->cmd, m, &dump);

    s = "unk";
    if(h->cmd < nelem(ctab))
        s = ctab[h->cmd];
    t = "unk";
    if(h->err < nelem(etab))
        s = etab[h->err];
    m->p = seprint(m->p, m->e, "cmd=%d %s err=%d %s cnt=%d\n",
        h->cmd, s, h->err, t, h->cnt);
    return 0;
}
/*e: function p_seprint (networking/ip/snoopy/aoemask.c) */

/*s: global aoemask */
Proto aoemask = {
    "aoemask",
    p_compile,
    p_filter,
    p_seprint,
    p_mux,
    "%lud",
    p_fields,
    defaultframer,
};
/*e: global aoemask */
/*e: networking/ip/snoopy/aoemask.c */
