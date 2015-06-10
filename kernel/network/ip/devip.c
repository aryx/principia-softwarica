/*s: kernel/network/ip/devip.c */
#include    "u.h"
#include    "../port/lib.h"
#include    "mem.h"
#include    "dat.h"
#include    "fns.h"
#include    "../port/error.h"
#include    "../ip/ip.h"

/*s: enum qid (kernel/network/ip/devip.c) */
enum
{
    Qtopdir=    1,      /* top level directory */

    Qtopbase,
    Qarp=       Qtopbase,
    Qiproute,
    /*s: [[Qid]] toplevel extra cases */
    Qipselftab,
    /*x: [[Qid]] toplevel extra cases */
    Qndb,
    /*x: [[Qid]] toplevel extra cases */
    Qlog,
    /*e: [[Qid]] toplevel extra cases */

    Qprotodir,          /* directory for a protocol */
    Qprotobase,
    Qclone=     Qprotobase,
    /*s: [[Qid]] protocol extra cases */
    Qstats,
    /*e: [[Qid]] protocol extra cases */

    Qconvdir,           /* directory for a conversation */
    Qconvbase,
    Qctl=       Qconvbase,
    Qdata,
    /*s: [[Qid]] conversation extra cases */
    Qerr,
    /*x: [[Qid]] conversation extra cases */
    Qlisten,
    /*x: [[Qid]] conversation extra cases */
    Qstatus,
    /*x: [[Qid]] conversation extra cases */
    Qlocal,
    Qremote,
    /*e: [[Qid]] conversation extra cases */
    /*s: [[Qid]] conversation extra cases, last entry */
    Qsnoop,
    /*e: [[Qid]] conversation extra cases, last entry */
};
/*e: enum qid (kernel/network/ip/devip.c) */

/*s: enum misc (kernel/network/ip/devip.c) */
enum
{
    Logtype=    5,
    Masktype=   (1<<Logtype)-1,

    Logconv=    12,
    Maskconv=   (1<<Logconv)-1,
    Shiftconv=  Logtype,

    Logproto=   8,
    Maskproto=  (1<<Logproto)-1,
    Shiftproto= Logtype + Logconv,

    /*s: constant Nfs */
    Nfs=        128,
    /*e: constant Nfs */
};
/*e: enum misc (kernel/network/ip/devip.c) */

/*s: macro TYPE */
#define TYPE(x)     ( ((ulong)(x).path) & Masktype )
/*e: macro TYPE */
/*s: macro CONV */
#define CONV(x)     ( (((ulong)(x).path) >> Shiftconv) & Maskconv )
/*e: macro CONV */
/*s: macro PROTO */
#define PROTO(x)    ( (((ulong)(x).path) >> Shiftproto) & Maskproto )
/*e: macro PROTO */
/*s: macro QID */
#define QID(p, cv, y)    ( ((p)<<(Shiftproto)) | ((cv)<<Shiftconv) | (y) )
/*e: macro QID */

/*s: global network */
static char network[] = "network";
/*e: global network */

/*s: global fslock */
QLock   fslock;
/*e: global fslock */
/*s: global ipfs */
Fs  *ipfs[Nfs]; /* attached fs's */
/*e: global ipfs */
//Queue *qlog;

extern  void nullmediumlink(void);
extern  void pktmediumlink(void);
long ndbwrite(Fs *f, char *a, ulong off, int n);
Conv*   Fsprotoclone(Proto*, char*);
char*   Fsstdbind(Conv*, char**, int);
void    closeconv(Conv*);

/*s: function ip3gen */
static int
ip3gen(Chan *c, int i, Dir *dp)
{
    Qid q;
    Conv *cv;
    char *p;

    cv = ipfs[c->dev]->p[PROTO(c->qid)]->conv[CONV(c->qid)];
    if(cv->owner == nil)
        kstrdup(&cv->owner, eve);
    mkqid(&q, QID(PROTO(c->qid), CONV(c->qid), i), 0, QTFILE);
    switch(i) {
    /*s: [[ip3gen()]] switch TYPE qid cases */
    case Qctl:
        devdir(c, q, "ctl", 0, cv->owner, cv->perm, dp);
        return 1;
    case Qdata:
        devdir(c, q, "data", qlen(cv->rq), cv->owner, cv->perm, dp);
        return 1;
    /*x: [[ip3gen()]] switch TYPE qid cases */
    case Qerr:
        devdir(c, q, "err", qlen(cv->eq), cv->owner, cv->perm, dp);
        return 1;
    /*x: [[ip3gen()]] switch TYPE qid cases */
    case Qlisten:
        devdir(c, q, "listen", 0, cv->owner, cv->perm, dp);
        return 1;
    /*x: [[ip3gen()]] switch TYPE qid cases */
    case Qstatus:
        p = "status";
        break;
    /*x: [[ip3gen()]] switch TYPE qid cases */
    case Qlocal:
        p = "local";
        break;
    case Qremote:
        p = "remote";
        break;
    /*x: [[ip3gen()]] switch TYPE qid cases */
    case Qsnoop:
        if(strcmp(cv->p->name, "ipifc") != 0)
            return -1;
        devdir(c, q, "snoop", qlen(cv->sq), cv->owner, 0400, dp);
        return 1;
    /*e: [[ip3gen()]] switch TYPE qid cases */
    default:
        return -1;
    }
    devdir(c, q, p, 0, cv->owner, 0444, dp);
    return 1;
}
/*e: function ip3gen */

/*s: function ip2gen */
static int
ip2gen(Chan *c, int i, Dir *dp)
{
    Qid q;

    switch(i) {
    case Qclone:
        mkqid(&q, QID(PROTO(c->qid), 0, Qclone), 0, QTFILE);
        devdir(c, q, "clone", 0, network, 0666, dp);
        return 1;
    case Qstats:
        mkqid(&q, QID(PROTO(c->qid), 0, Qstats), 0, QTFILE);
        devdir(c, q, "stats", 0, network, 0444, dp);
        return 1;
    }
    return -1;
}
/*e: function ip2gen */

/*s: function ip1gen */
static int
ip1gen(Chan *c, int i, Dir *dp)
{
    Fs *f;
    Qid q;
    int prot;
    int len = 0;
    char *p;
    /*s: [[ip1gen()]] locals */
    extern ulong    kerndate;
    /*e: [[ip1gen()]] locals */

    f = ipfs[c->dev];
    prot = 0666;
    mkqid(&q, QID(0, 0, i), 0, QTFILE);
    switch(i) {
    /*s: [[ip1gen()]] switch TYPE qid cases */
    case Qarp:
        p = "arp";
        prot = 0664;
        break;
    /*x: [[ip1gen()]] switch TYPE qid cases */
    case Qipselftab:
        p = "ipselftab";
        prot = 0444;
        break;
    /*x: [[ip1gen()]] switch TYPE qid cases */
    case Qiproute:
        p = "iproute";
        prot = 0664;
        break;
    /*x: [[ip1gen()]] switch TYPE qid cases */
    case Qndb:
        p = "ndb";
        len = strlen(f->ndb);
        q.vers = f->ndbvers;
        break;
    /*x: [[ip1gen()]] switch TYPE qid cases */
    case Qlog:
        p = "log";
        break;
    /*e: [[ip1gen()]] switch TYPE qid cases */
    default:
        return -1;
    }
    devdir(c, q, p, len, network, prot, dp);
    /*s: [[ipgen()]] if Qndb, adjust mtime */
    if(i == Qndb && f->ndbmtime > kerndate)
        dp->mtime = f->ndbmtime;
    /*e: [[ipgen()]] if Qndb, adjust mtime */
    return 1;
}
/*e: function ip1gen */

/*s: function ipgen */
static int
ipgen(Chan *c, char*, Dirtab*, int, int s, Dir *dp)
{
    Fs *f;
    Qid q;
    Conv *cv;

    f = ipfs[c->dev];
    switch(TYPE(c->qid)) {
    /*s: [[ipgen()]] switch TYPE qid cases */
    case Qtopdir:
        if(s == DEVDOTDOT){
            mkqid(&q, QID(0, 0, Qtopdir), 0, QTDIR);
            snprint(up->genbuf, sizeof up->genbuf, "#I%lud", c->dev);
            devdir(c, q, up->genbuf, 0, network, 0555, dp);
            return 1;
        }
        if(s < f->np) {
            if(f->p[s]->connect == nil)
                return 0;   /* protocol with no user interface */
            mkqid(&q, QID(s, 0, Qprotodir), 0, QTDIR);
            devdir(c, q, f->p[s]->name, 0, network, 0555, dp);
            return 1;
        }
        s -= f->np;
        return ip1gen(c, Qtopbase+s, dp);
    /*x: [[ipgen()]] switch TYPE qid cases */
    case Qarp:
    case Qiproute:
    case Qipselftab:
    case Qndb:
    case Qlog:
        return ip1gen(c, TYPE(c->qid), dp);
    /*x: [[ipgen()]] switch TYPE qid cases */
    case Qprotodir:
        if(s == DEVDOTDOT){
            mkqid(&q, QID(0, 0, Qtopdir), 0, QTDIR);
            snprint(up->genbuf, sizeof up->genbuf, "#I%lud", c->dev);
            devdir(c, q, up->genbuf, 0, network, 0555, dp);
            return 1;
        }
        if(s < f->p[PROTO(c->qid)]->ac) {
            cv = f->p[PROTO(c->qid)]->conv[s];
            snprint(up->genbuf, sizeof up->genbuf, "%d", s);
            mkqid(&q, QID(PROTO(c->qid), s, Qconvdir), 0, QTDIR);
            devdir(c, q, up->genbuf, 0, cv->owner, 0555, dp);
            return 1;
        }
        s -= f->p[PROTO(c->qid)]->ac;
        return ip2gen(c, s+Qprotobase, dp);
    /*x: [[ipgen()]] switch TYPE qid cases */
    case Qclone:
    case Qstats:
        return ip2gen(c, TYPE(c->qid), dp);
    /*x: [[ipgen()]] switch TYPE qid cases */
    case Qconvdir:
        if(s == DEVDOTDOT){
            s = PROTO(c->qid);
            mkqid(&q, QID(s, 0, Qprotodir), 0, QTDIR);
            devdir(c, q, f->p[s]->name, 0, network, 0555, dp);
            return 1;
        }
        return ip3gen(c, s+Qconvbase, dp);
    /*x: [[ipgen()]] switch TYPE qid cases */
    case Qctl:
    case Qdata:
    case Qerr:
    case Qlisten:
    case Qlocal:
    case Qremote:
    case Qstatus:
    case Qsnoop:
        return ip3gen(c, TYPE(c->qid), dp);
    /*e: [[ipgen()]] switch TYPE qid cases */
    }
    return -1;
}
/*e: function ipgen */

/*s: function ipreset */
static void
ipreset(void)
{
    nullmediumlink();
    pktmediumlink();

    fmtinstall('i', eipfmt);
    fmtinstall('I', eipfmt);
    fmtinstall('E', eipfmt);
    fmtinstall('V', eipfmt);
    fmtinstall('M', eipfmt);
}
/*e: function ipreset */

/*s: function ipgetfs */
static Fs*
ipgetfs(int dev)
{
    extern void (*ipprotoinit[])(Fs*);
    Fs *f;
    int i;

    if(dev >= Nfs)
        return nil;

    qlock(&fslock);
    if(ipfs[dev] == nil){
        f = smalloc(sizeof(Fs));

        ip_init(f);
        arpinit(f);
        netloginit(f);
        for(i = 0; ipprotoinit[i]; i++)
            ipprotoinit[i](f);

        f->dev = dev;
        ipfs[dev] = f;
    }
    qunlock(&fslock);

    return ipfs[dev];
}
/*e: function ipgetfs */

/*s: function newipaux */
IPaux*
newipaux(char *owner, char *tag)
{
    IPaux *a;
    int n;

    a = smalloc(sizeof(IPaux));

    kstrdup(&a->owner, owner);
    memset(a->tag, ' ', sizeof(a->tag));
    n = strlen(tag);
    if(n > sizeof(a->tag))
        n = sizeof(a->tag);
    memmove(a->tag, tag, n);

    return a;
}
/*e: function newipaux */

/*s: macro ATTACHER */
#define ATTACHER(c) (((IPaux*)((c)->aux))->owner)
/*e: macro ATTACHER */

/*s: function ipattach */
static Chan*
ipattach(char* spec)
{
    Chan *c;
    int dev;

    dev = atoi(spec);
    if(dev >= Nfs)
        error("bad specification");

    // initialize ip stack
    ipgetfs(dev);

    c = devattach('I', spec);
    mkqid(&c->qid, QID(0, 0, Qtopdir), 0, QTDIR);
    c->dev = dev;
    c->aux = newipaux(up->user, "none");

    return c;
}
/*e: function ipattach */

/*s: function ipwalk */
static Walkqid*
ipwalk(Chan* c, Chan *nc, char **name, int nname)
{
    IPaux *a = c->aux;
    Walkqid* w;

    w = devwalk(c, nc, name, nname, nil, 0, ipgen);

    if(w != nil && w->clone != nil)
        w->clone->aux = newipaux(a->owner, a->tag);

    return w;
}
/*e: function ipwalk */


/*s: function ipstat */
static int
ipstat(Chan* c, uchar* db, int n)
{
    return devstat(c, db, n, nil, 0, ipgen);
}
/*e: function ipstat */

/*s: function incoming */
static int
incoming(void* arg)
{
    Conv *conv;

    conv = arg;
    return conv->incall != nil;
}
/*e: function incoming */

/*s: global m2p */
static int m2p[] = {
    [OREAD]     4,
    [OWRITE]    2,
    [ORDWR]     6
};
/*e: global m2p */

/*s: function ipopen */
static Chan*
ipopen(Chan* c, int omode)
{
    Fs *f;
    int perm;
    Proto *p;
    Conv *cv, *nc;

    f = ipfs[c->dev];
    perm = m2p[omode&3];
    switch(TYPE(c->qid)) {
    /*s: [[ipopen()]] switch TYPE qid cases */
    case Qtopdir:
    case Qprotodir:
    case Qconvdir:

    case Qipselftab:

    case Qstatus:
    case Qlocal:
    case Qremote:
    case Qstats:
        if(omode != OREAD)
            error(Eperm);
        break;
    /*x: [[ipopen()]] switch TYPE qid cases */
    case Qctl:
    case Qdata:
    case Qerr:
        p = f->p[PROTO(c->qid)];
        qlock(p);
        cv = p->conv[CONV(c->qid)];
        qlock(cv);
        if(waserror()) {
            qunlock(cv);
            qunlock(p);
            nexterror();
        }
        if((perm & (cv->perm>>6)) != perm) {
            if(strcmp(ATTACHER(c), cv->owner) != 0)
                error(Eperm);
            if((perm & cv->perm) != perm)
                error(Eperm);

        }
        cv->inuse++;
        if(cv->inuse == 1){
            kstrdup(&cv->owner, ATTACHER(c));
            cv->perm = 0660;
        }
        qunlock(cv);
        qunlock(p);
        poperror();
        break;
    /*x: [[ipopen()]] switch TYPE qid cases */
    case Qclone:
        p = f->p[PROTO(c->qid)];

        qlock(p);
        if(waserror()){
            qunlock(p);
            nexterror();
        }

        cv = Fsprotoclone(p, ATTACHER(c));

        qunlock(p);
        poperror();
        if(cv == nil) {
            error(Enodev);
            break;
        }
        mkqid(&c->qid, QID(p->x, cv->x, Qctl), 0, QTFILE);
        break;
    /*x: [[ipopen()]] switch TYPE qid cases */
    case Qlisten:
        cv = f->p[PROTO(c->qid)]->conv[CONV(c->qid)];
        if((perm & (cv->perm>>6)) != perm) {
            if(strcmp(ATTACHER(c), cv->owner) != 0)
                error(Eperm);
            if((perm & cv->perm) != perm)
                error(Eperm);

        }

        if(cv->state != Announced)
            error("not announced");

        if(waserror()){
            closeconv(cv);
            nexterror();
        }
        qlock(cv);
        cv->inuse++;
        qunlock(cv);

        nc = nil;
        while(nc == nil) {
            /* give up if we got a hangup */
            if(qisclosed(cv->rq))
                error("listen hungup");

            qlock(&cv->listenq);
            if(waserror()) {
                qunlock(&cv->listenq);
                nexterror();
            }

            /* wait for a connect */
            sleep(&cv->listenr, incoming, cv);

            qlock(cv);
            nc = cv->incall;
            if(nc != nil){
                cv->incall = nc->next;
                mkqid(&c->qid, QID(PROTO(c->qid), nc->x, Qctl), 0, QTFILE);
                kstrdup(&cv->owner, ATTACHER(c));
            }
            qunlock(cv);

            qunlock(&cv->listenq);
            poperror();
        }
        closeconv(cv);
        poperror();
        break;
    /*x: [[ipopen()]] switch TYPE qid cases */
    case Qarp:
    case Qiproute:
        if(omode != OREAD && !iseve())
            error(Eperm);
        break;
    /*x: [[ipopen()]] switch TYPE qid cases */
    case Qndb:
        if(omode & (OWRITE|OTRUNC) && !iseve())
            error(Eperm);
        if((omode & (OWRITE|OTRUNC)) == (OWRITE|OTRUNC))
            f->ndb[0] = 0;
        break;
    /*x: [[ipopen()]] switch TYPE qid cases */
    case Qlog:
        netlogopen(f);
        break;
    /*x: [[ipopen()]] switch TYPE qid cases */
    case Qsnoop:
        if(omode != OREAD)
            error(Eperm);
        p = f->p[PROTO(c->qid)];
        cv = p->conv[CONV(c->qid)];
        if(strcmp(ATTACHER(c), cv->owner) != 0 && !iseve())
            error(Eperm);
        incref(&cv->snoopers);
        break;
    /*e: [[ipopen()]] switch TYPE qid cases */
    default:
        break;
    }
    c->mode = openmode(omode);
    c->flag |= COPEN;
    c->offset = 0;
    return c;
}
/*e: function ipopen */

/*s: function ipcreate */
static void
ipcreate(Chan*, char*, int, ulong)
{
    error(Eperm);
}
/*e: function ipcreate */

/*s: function ipremove */
static void
ipremove(Chan*)
{
    error(Eperm);
}
/*e: function ipremove */

/*s: function ipwstat */
static int
ipwstat(Chan *c, uchar *dp, int n)
{
    Dir d;
    Conv *cv;
    Fs *f;
    Proto *p;

    f = ipfs[c->dev];
    switch(TYPE(c->qid)) {
    default:
        error(Eperm);
        break;
    case Qctl:
    case Qdata:
        break;
    }

    n = convM2D(dp, n, &d, nil);
    if(n > 0){
        p = f->p[PROTO(c->qid)];
        cv = p->conv[CONV(c->qid)];
        if(!iseve() && strcmp(ATTACHER(c), cv->owner) != 0)
            error(Eperm);
        if(d.uid[0])
            kstrdup(&cv->owner, d.uid);
        cv->perm = d.mode & 0777;
    }
    return n;
}
/*e: function ipwstat */

/*s: function closeconv */
void
closeconv(Conv *cv)
{
    Conv *nc;
    /*s: [[closeconv()]] locals */
    Ipmulti *mp;
    /*e: [[closeconv()]] locals */

    qlock(cv);

    if(--cv->inuse > 0) {
        qunlock(cv);
        return;
    }
    /*s: [[closeconv()]] close incoming calls */
    /* close all incoming calls since no listen will ever happen */
    for(nc = cv->incall; nc; nc = cv->incall){
        cv->incall = nc->next;
        closeconv(nc);
    }
    cv->incall = nil;
    /*e: [[closeconv()]] close incoming calls */

    kstrdup(&cv->owner, network);
    cv->perm = 0660;
    /*s: [[closeconv()]] if multi, call ipifcremmulti */
    while((mp = cv->multi) != nil)
        ipifcremmulti(cv, mp->ma, mp->ia);
    /*e: [[closeconv()]] if multi, call ipifcremmulti */
    cv->r = nil;
    cv->rgen = 0;
    // Protocol dispatch
    cv->p->close(cv);
    cv->state = Idle;

    qunlock(cv);
}
/*e: function closeconv */

/*s: function ipclose */
static void
ipclose(Chan* c)
{
    Fs *f;

    f = ipfs[c->dev];
    switch(TYPE(c->qid)) {
    /*s: [[ipclose()]] switch TYPE qid cases */
    case Qctl:
    case Qdata:
    case Qerr:
        if(c->flag & COPEN)
            closeconv(f->p[PROTO(c->qid)]->conv[CONV(c->qid)]);
        break;
    /*x: [[ipclose()]] switch TYPE qid cases */
    case Qlog:
        if(c->flag & COPEN)
            netlogclose(f);
        break;
    /*x: [[ipclose()]] switch TYPE qid cases */
    case Qsnoop:
        if(c->flag & COPEN)
            decref(&f->p[PROTO(c->qid)]->conv[CONV(c->qid)]->snoopers);
        break;
    /*e: [[ipclose()]] switch TYPE qid cases */
    default:
        break;
    }
    free(((IPaux*)c->aux)->owner);
    free(c->aux);
}
/*e: function ipclose */

/*s: enum _anon_ (kernel/network/ip/devip.c)2 */
enum
{
    /*s: constant Statelen */
        Statelen=   32*1024,
    /*e: constant Statelen */
};
/*e: enum _anon_ (kernel/network/ip/devip.c)2 */

/*s: function ipread */
static long
ipread(Chan *ch, void *a, long n, vlong off)
{
    Fs *f;
    Proto *x;
    Conv *cv;
    char *buf, *p;
    long rv;
    ulong offset = off;

    f = ipfs[ch->dev];
    p = a;
    switch(TYPE(ch->qid)) {
    /*s: [[ipread()]] switch TYPE qid cases */
    case Qtopdir:
    case Qprotodir:
    case Qconvdir:
        return devdirread(ch, a, n, 0, 0, ipgen);
    /*x: [[ipread()]] switch TYPE qid cases */
    case Qctl:
        buf = smalloc(16);
        snprint(buf, 16, "%lud", CONV(ch->qid));
        rv = readstr(offset, p, n, buf);
        free(buf);
        return rv;
    /*x: [[ipread()]] switch TYPE qid cases */
    case Qdata:
        cv = f->p[PROTO(ch->qid)]->conv[CONV(ch->qid)];
        return qread(cv->rq, a, n);
    /*x: [[ipread()]] switch TYPE qid cases */
    case Qerr:
        cv = f->p[PROTO(ch->qid)]->conv[CONV(ch->qid)];
        return qread(cv->eq, a, n);
    /*x: [[ipread()]] switch TYPE qid cases */
    case Qstats:
        x = f->p[PROTO(ch->qid)];
        if(x->stats == nil)
            error("stats not implemented");
        buf = smalloc(Statelen);

        // Protocol dispatch
        (*x->stats)(x, buf, Statelen);

        rv = readstr(offset, p, n, buf);
        free(buf);
        return rv;
    /*x: [[ipread()]] switch TYPE qid cases */
    case Qstatus:
        buf = smalloc(Statelen);
        x = f->p[PROTO(ch->qid)];
        cv = x->conv[CONV(ch->qid)];

        // Protocol dispatch
        (*x->state)(cv, buf, Statelen-2);

        rv = readstr(offset, p, n, buf);
        free(buf);
        return rv;
    /*x: [[ipread()]] switch TYPE qid cases */
    case Qlocal:
        buf = smalloc(Statelen);
        x = f->p[PROTO(ch->qid)];
        cv = x->conv[CONV(ch->qid)];
        if(x->local == nil) {
            snprint(buf, Statelen, "%I!%d\n", cv->laddr, cv->lport);
        } else {
            // Protocol dispatch
            (*x->local)(cv, buf, Statelen-2);
        }
        rv = readstr(offset, p, n, buf);
        free(buf);
        return rv;
    /*x: [[ipread()]] switch TYPE qid cases */
    case Qremote:
        buf = smalloc(Statelen);
        x = f->p[PROTO(ch->qid)];
        cv = x->conv[CONV(ch->qid)];
        if(x->remote == nil) {
            snprint(buf, Statelen, "%I!%d\n", cv->raddr, cv->rport);
        } else {
            // Protocol dispatch
            (*x->remote)(cv, buf, Statelen-2);
        }
        rv = readstr(offset, p, n, buf);
        free(buf);
        return rv;
    /*x: [[ipread()]] switch TYPE qid cases */
    case Qarp:
        return arpread(f->arp, a, offset, n);
    /*x: [[ipread()]] switch TYPE qid cases */
    case Qipselftab:
        return ipselftabread(f, a, offset, n);
    /*x: [[ipread()]] switch TYPE qid cases */
    case Qiproute:
        return routeread(f, a, offset, n);
    /*x: [[ipread()]] switch TYPE qid cases */
    case Qndb:
        return readstr(offset, a, n, f->ndb);
    /*x: [[ipread()]] switch TYPE qid cases */
    case Qlog:
        return netlogread(f, a, offset, n);
    /*x: [[ipread()]] switch TYPE qid cases */
    case Qsnoop:
        cv = f->p[PROTO(ch->qid)]->conv[CONV(ch->qid)];
        return qread(cv->sq, a, n);
    /*e: [[ipread()]] switch TYPE qid cases */
    default:
        error(Eperm);

    }
}
/*e: function ipread */

/*s: function ipbread */
static Block*
ipbread(Chan* ch, long n, ulong offset)
{
    Conv *c;
    Proto *x;
    Fs *f;

    switch(TYPE(ch->qid)){
    case Qdata:
        f = ipfs[ch->dev];
        x = f->p[PROTO(ch->qid)];
        c = x->conv[CONV(ch->qid)];
        return qbread(c->rq, n);
    default:
        return devbread(ch, n, offset);
    }
}
/*e: function ipbread */

/*s: function setladdr */
/*
 *  set local address to be that of the ifc closest to remote address
 */
static void
setladdr(Conv* c)
{
    findlocalip(c->p->f, c->laddr, c->raddr);
}
/*e: function setladdr */

/*s: function setluniqueport */
/*
 *  set a local port making sure the quad of raddr,rport,laddr,lport is unique
 */
char*
setluniqueport(Conv* c, int lport)
{
    Proto *p;
    Conv *xp;
    int x;

    p = c->p;

    qlock(p);
    for(x = 0; x < p->nc; x++){
        xp = p->conv[x];
        if(xp == nil)
            break;
        if(xp == c)
            continue;
        if((xp->state == Connected || xp->state == Announced)
        && xp->lport == lport
        && xp->rport == c->rport
        && ipcmp(xp->raddr, c->raddr) == 0
        && ipcmp(xp->laddr, c->laddr) == 0){
            qunlock(p);
            return "address in use";
        }
    }
    c->lport = lport;
    qunlock(p);
    return nil;
}
/*e: function setluniqueport */

/*s: function lportinuse */
/*
 * is lport in use by anyone?
 */
static int
lportinuse(Proto *p, ushort lport)
{
    int x;

    for(x = 0; x < p->nc && p->conv[x]; x++)
        if(p->conv[x]->lport == lport)
            return 1;
    return 0;
}
/*e: function lportinuse */

/*s: function setlport */
/*
 *  pick a local port and set it
 */
char *
setlport(Conv* c)
{
    Proto *p;
    int i, port;

    p = c->p;
    qlock(p);
    if(c->restricted){
        /* Restricted ports cycle between 600 and 1024. */
        for(i=0; i<1024-600; i++){
            if(p->nextrport >= 1024 || p->nextrport < 600)
                p->nextrport = 600;
            port = p->nextrport++;
            if(!lportinuse(p, port))
                goto chosen;
        }
    }else{
        /*
         * Unrestricted ports are chosen randomly
         * between 2^15 and 2^16.  There are at most
         * 4*Nchan = 4096 ports in use at any given time,
         * so even in the worst case, a random probe has a
         * 1 - 4096/2^15 = 87% chance of success.
         * If 64 successive probes fail, there is a bug somewhere
         * (or a once in 10^58 event has happened, but that's
         * less likely than a venti collision).
         */
        for(i=0; i<64; i++){
            port = (1<<15) + nrand(1<<15);
            if(!lportinuse(p, port))
                goto chosen;
        }
    }
    qunlock(p);
    /*
     * debugging: let's see if we ever get this.
     * if we do (and we're a cpu server), we might as well restart
     * since we're now unable to service new connections.
     */
    panic("setlport: out of ports");
    return "no ports available";

chosen:
    c->lport = port;
    qunlock(p);
    return nil;
}
/*e: function setlport */

/*s: function setladdrport */
/*
 *  set a local address and port from a string of the form
 *  [address!]port[!r]
 */
char*
setladdrport(Conv* c, char* str, int announcing)
{
    char *p;
    char *rv;
    ushort lport;
    uchar addr[IPaddrlen];

    /*
     *  ignore restricted part if it exists.  it's
     *  meaningless on local ports.
     */
    p = strchr(str, '!');
    if(p != nil){
        *p++ = 0;
        if(strcmp(p, "r") == 0)
            p = nil;
    }

    c->lport = 0;
    if(p == nil){
        if(announcing)
            ipmove(c->laddr, IPnoaddr);
        else
            setladdr(c);
        p = str;
    } else {
        if(strcmp(str, "*") == 0)
            ipmove(c->laddr, IPnoaddr);
        else {
            if(parseip(addr, str) == -1)
                return Ebadip;
            if(ipforme(c->p->f, addr))
                ipmove(c->laddr, addr);
            else
                return "not a local IP address";
        }
    }

    /* one process can get all connections */
    if(announcing && strcmp(p, "*") == 0){
        if(!iseve())
            error(Eperm);
        return setluniqueport(c, 0);
    }

    lport = atoi(p);
    if(lport <= 0)
        rv = setlport(c);
    else
        rv = setluniqueport(c, lport);
    return rv;
}
/*e: function setladdrport */

/*s: function setraddrport */
static char*
setraddrport(Conv* c, char* str)
{
    char *p;

    p = strchr(str, '!');
    if(p == nil)
        return "malformed address";
    *p++ = 0;
    if (parseip(c->raddr, str) == -1)
        return Ebadip;
    c->rport = atoi(p);
    p = strchr(p, '!');
    if(p){
        if(strstr(p, "!r") != nil)
            c->restricted = true;
    }
    return nil;
}
/*e: function setraddrport */

/*s: function Fsstdconnect */
/*
 *  called by protocol connect routine to set addresses
 */
char*
Fsstdconnect(Conv *c, char *argv[], int argc)
{
    char *err;

    switch(argc) {
    case 2:
        err = setraddrport(c, argv[1]);
        if(err != nil)
            return err;
        setladdr(c);
        err = setlport(c);
        if (err != nil)
            return err;
        break;
    case 3:
        err = setraddrport(c, argv[1]);
        if(err != nil)
            return err;
        err = setladdrport(c, argv[2], 0);
        if(err != nil)
            return err;
    default:
        return "bad args to connect";
    }

    /*s: [[Fsstdconnect()]] set ipversion field to V4 or V6 */
    if( (memcmp(c->raddr, v4prefix, IPv4off) == 0 &&
         memcmp(c->laddr, v4prefix, IPv4off) == 0)
        || ipcmp(c->raddr, IPnoaddr) == 0)
        c->ipversion = V4;
    else
        c->ipversion = V6;
    /*e: [[Fsstdconnect()]] set ipversion field to V4 or V6 */

    return nil;
}
/*e: function Fsstdconnect */
/*s: function connected */
/*
 *  initiate connection and sleep till its set up
 */
static bool
connected(void* a)
{
    return ((Conv*)a)->state == Connected;
}
/*e: function connected */
/*s: function connectctlmsg */
static void
connectctlmsg(Proto *p, Conv *c, Cmdbuf *cb)
{
    char *err;

    if(c->state != Idle)
        error(Econinuse);

    c->state = Connecting;
    c->cerr[0] = '\0';

    if(p->connect == nil)
        error("connect not supported");
    // Protocol dispatch
    err = p->connect(c, cb->f, cb->nf);
    if(err != nil)
        error(err);

    qunlock(c);
    if(waserror()){
        qlock(c);
        nexterror();
    }
    sleep(&c->cr, connected, c);
    qlock(c);
    poperror();

    if(c->cerr[0] != '\0')
        error(c->cerr);
}
/*e: function connectctlmsg */

/*s: function Fsstdannounce */
/*
 *  called by protocol announce routine to set addresses
 */
char*
Fsstdannounce(Conv* c, char* argv[], int argc)
{
    memset(c->raddr, 0, sizeof(c->raddr));
    c->rport = 0;
    switch(argc){
    default:
        break;
    case 2:
        return setladdrport(c, argv[1], 1);
    }
    return "bad args to announce";
}
/*e: function Fsstdannounce */

/*s: function announced */
/*
 *  initiate announcement and sleep till its set up
 */
static bool
announced(void* a)
{
    return ((Conv*)a)->state == Announced;
}
/*e: function announced */
/*s: function announcectlmsg */
static void
announcectlmsg(Proto *p, Conv *c, Cmdbuf *cb)
{
    char *err;

    if(c->state != Idle)
        error(Econinuse);

    c->state = Announcing;
    c->cerr[0] = '\0';

    if(p->announce == nil)
        error("announce not supported");
    // Protocol dispatch
    err = p->announce(c, cb->f, cb->nf);

    if(err != nil)
        error(err);

    qunlock(c);
    if(waserror()){
        qlock(c);
        nexterror();
    }
    sleep(&c->cr, announced, c);
    qlock(c);
    poperror();

    if(c->cerr[0] != '\0')
        error(c->cerr);
}
/*e: function announcectlmsg */

/*s: function Fsstdbind */
/*
 *  called by protocol bind routine to set addresses
 */
char*
Fsstdbind(Conv* cv, char* argv[], int argc)
{
    switch(argc){
    default:
        break;
    case 2:
        return setladdrport(cv, argv[1], 0);
    }
    return "bad args to bind";
}
/*e: function Fsstdbind */

/*s: function bindctlmsg */
static void
bindctlmsg(Proto *x, Conv *cv, Cmdbuf *cb)
{
    char *p;

    if(x->bind == nil)
        p = Fsstdbind(cv, cb->f, cb->nf);
    else
        // Protocol dispatch
        p = x->bind(cv, cb->f, cb->nf);
    if(p != nil)
        error(p);
}
/*e: function bindctlmsg */

/*s: function tosctlmsg */
static void
tosctlmsg(Conv *c, Cmdbuf *cb)
{
    if(cb->nf < 2)
        c->tos = 0;
    else
        c->tos = atoi(cb->f[1]);
}
/*e: function tosctlmsg */

/*s: function ttlctlmsg */
static void
ttlctlmsg(Conv *c, Cmdbuf *cb)
{
    if(cb->nf < 2)
        c->ttl = MAXTTL;
    else
        c->ttl = atoi(cb->f[1]);
}
/*e: function ttlctlmsg */

/*s: function ipwrite */
static long
ipwrite(Chan* ch, void *v, long n, vlong off)
{
    Fs *f;
    Proto *x;
    Conv *cv;
    char *p; // err?
    Cmdbuf *cb;
    char *a;
    ulong offset = off;
    /*s: [[ipwrite()]] locals */
    uchar ia[IPaddrlen], ma[IPaddrlen];
    /*e: [[ipwrite()]] locals */

    f = ipfs[ch->dev];
    a = v;
    switch(TYPE(ch->qid)){
    /*s: [[ipwrite()]] switch TYPE qid cases */
    case Qctl:
        x = f->p[PROTO(ch->qid)];
        cv = x->conv[CONV(ch->qid)];
        cb = parsecmd(a, n);

        qlock(cv);
        if(waserror()) {
            qunlock(cv);
            free(cb);
            nexterror();
        }
        if(cb->nf < 1)
            error("short control request");

        /*s: [[ipwrite()]] Qctl case, if connect string */
        if(strcmp(cb->f[0], "connect") == 0)
            connectctlmsg(x, cv, cb);
        /*e: [[ipwrite()]] Qctl case, if connect string */
        /*s: [[ipwrite()]] Qctl case, else if announce string */
        else if(strcmp(cb->f[0], "announce") == 0)
            announcectlmsg(x, cv, cb);
        /*e: [[ipwrite()]] Qctl case, else if announce string */
        /*s: [[ipwrite()]] Qctl case, else if other string */
        else if(strcmp(cb->f[0], "bind") == 0)
            bindctlmsg(x, cv, cb);
        /*x: [[ipwrite()]] Qctl case, else if other string */
        else if(strcmp(cb->f[0], "tos") == 0)
            tosctlmsg(cv, cb);
        /*x: [[ipwrite()]] Qctl case, else if other string */
        else if(strcmp(cb->f[0], "ttl") == 0)
            ttlctlmsg(cv, cb);
        /*x: [[ipwrite()]] Qctl case, else if other string */
        else if(strcmp(cb->f[0], "maxfragsize") == 0){
            if(cb->nf < 2)
                error("maxfragsize needs size");

            cv->maxfragsize = (int)strtol(cb->f[1], nil, 0);

        } 
        /*x: [[ipwrite()]] Qctl case, else if other string */
        else if(strcmp(cb->f[0], "ignoreadvice") == 0)
            cv->ignoreadvice = true;
        /*x: [[ipwrite()]] Qctl case, else if other string */
        else if(strcmp(cb->f[0], "addmulti") == 0){
            if(cb->nf < 2)
                error("addmulti needs interface address");
            if(cb->nf == 2){
                if(!ipismulticast(cv->raddr))
                    error("addmulti for a non multicast address");
                if (parseip(ia, cb->f[1]) == -1)
                    error(Ebadip);
                ipifcaddmulti(cv, cv->raddr, ia);
            } else {
                if (parseip(ia, cb->f[1]) == -1 ||
                    parseip(ma, cb->f[2]) == -1)
                    error(Ebadip);
                if(!ipismulticast(ma))
                    error("addmulti for a non multicast address");
                ipifcaddmulti(cv, ma, ia);
            }
        } 
        /*x: [[ipwrite()]] Qctl case, else if other string */
        else if(strcmp(cb->f[0], "remmulti") == 0){
            if(cb->nf < 2)
                error("remmulti needs interface address");
            if(!ipismulticast(cv->raddr))
                error("remmulti for a non multicast address");
            if (parseip(ia, cb->f[1]) == -1)
                error(Ebadip);
            ipifcremmulti(cv, cv->raddr, ia);
        }
        /*e: [[ipwrite()]] Qctl case, else if other string */
        else if(x->ctl != nil) {
            // Protocol dispatch
            p = x->ctl(cv, cb->f, cb->nf);
            if(p != nil)
                error(p);
        } else
            error("unknown control request");
        qunlock(cv);
        free(cb);
        poperror();
        break;
    /*x: [[ipwrite()]] switch TYPE qid cases */
    case Qdata:
        cv = f->p[PROTO(ch->qid)]->conv[CONV(ch->qid)];
        if(cv->wq == nil)
            error(Eperm);
        qwrite(cv->wq, a, n);
        break;
    /*x: [[ipwrite()]] switch TYPE qid cases */
    case Qarp:
        return arpwrite(f, a, n);
    /*x: [[ipwrite()]] switch TYPE qid cases */
    case Qiproute:
        return routewrite(f, ch, a, n);
    /*x: [[ipwrite()]] switch TYPE qid cases */
    case Qndb:
        return ndbwrite(f, a, offset, n);
        break;
    /*x: [[ipwrite()]] switch TYPE qid cases */
    case Qlog:
        netlogctl(f, a, n);
        return n;
    /*e: [[ipwrite()]] switch TYPE qid cases */
    default:
        error(Eperm);
    }
    return n;
}
/*e: function ipwrite */

/*s: function ipbwrite */
static long
ipbwrite(Chan* ch, Block* bp, ulong offset)
{
    Conv *c;
    Proto *x;
    Fs *f;
    int n;

    switch(TYPE(ch->qid)){
    case Qdata:
        f = ipfs[ch->dev];
        x = f->p[PROTO(ch->qid)];
        c = x->conv[CONV(ch->qid)];

        if(c->wq == nil)
            error(Eperm);

        if(bp->next)
            bp = concatblock(bp);
        n = BLEN(bp);
        qbwrite(c->wq, bp);
        return n;
    default:
        return devbwrite(ch, bp, offset);
    }
}
/*e: function ipbwrite */

/*s: global ipdevtab */
Dev ipdevtab = {
    .dc       =    'I',
    .name     =    "ip",

    .attach   =    ipattach,
    .walk     =    ipwalk,
    .open     =    ipopen,
    .close    =    ipclose,
    .read     =    ipread,
    .write    =    ipwrite,
    .stat     =    ipstat,
    .wstat    =    ipwstat,
               
    .create   =    ipcreate,
    .remove   =    ipremove,
    .bread    =    ipbread,
    .bwrite   =    ipbwrite,
    .reset    =    ipreset,
    .init     =    devinit,
    .shutdown =    devshutdown,
};
/*e: global ipdevtab */

/*s: function Fsproto */
int
Fsproto(Fs *f, Proto *p)
{
    if(f->np >= Maxproto)
        return -1;

    p->f = f;

    /*s: [[Fsproto()]] adjust f->t2p */
    if(p->ipproto > 0){
        if(f->t2p[p->ipproto] != nil)
            return -1;
        f->t2p[p->ipproto] = p;
    }
    /*e: [[Fsproto()]] adjust f->t2p */

    p->conv = malloc(sizeof(Conv*) * (p->nc+1));
    if(p->conv == nil)
        panic("Fsproto");

    p->nextrport = 600;

    p->x = f->np;
    f->p[f->np++] = p;

    return 0;
}
/*e: function Fsproto */

/*s: function Fsprotoclone */
/*
 *  called with protocol locked
 */
Conv*
Fsprotoclone(Proto *p, char *user)
{
    Conv *cv, **pp, **ep;

retry:
    cv = nil;
    /*s: [[Fsprotoclone()]] finding an available conversation in the protocol */
    ep = &p->conv[p->nc];
    for(pp = p->conv; pp < ep; pp++) {
        cv = *pp;
        // found an unallocated entry in the array
        if(cv == nil){
            cv = malloc(sizeof(Conv));
            if(cv == nil)
                error(Enomem);
            qlock(cv);

            cv->p = p;
            cv->x = pp - p->conv;
            if(p->ptclsize != 0){
                cv->ptcl = malloc(p->ptclsize);
                if(cv->ptcl == nil) {
                    free(cv);
                    error(Enomem);
                }
            }
            *pp = cv;
            p->ac++;
            cv->eq = qopen(1024, Qmsg, 0, 0);

            // !! Protocol dispatch !!! will create extra queues
            (*p->create)(cv);

            break;
        }
        /*s: [[Fsprotoclone()]] if found an unused entry */
        if(canqlock(cv)){
            /*
             *  make sure both processes and protocol
             *  are done with this Conv
             */
            if(cv->inuse == 0 && (p->inuse == nil || 
                // Protocol dispatch
                (*p->inuse)(cv) == false)
              )
                break;

            qunlock(cv);
        }
        /*e: [[Fsprotoclone()]] if found an unused entry */
    }
    /*e: [[Fsprotoclone()]] finding an available conversation in the protocol */
    /*s: [[Fsprotoclone()]] if no more available conv, garbage collect and retry */
    if(pp >= ep) {
        if(p->gc)
            print("Fsprotoclone: garbage collecting Convs\n");
        if(p->gc != nil &&
            // Protocol dispatch
            (*p->gc)(p)
           )
            goto retry;

        /* debugging: do we ever get here? */
        if (cpuserver)
            panic("Fsprotoclone: all conversations in use");
        return nil;
    }
    /*e: [[Fsprotoclone()]] if no more available conv, garbage collect and retry */

    cv->inuse = 1;

    kstrdup(&cv->owner, user);
    cv->perm = 0660;

    cv->state = Idle;

    ipmove(cv->laddr, IPnoaddr);
    ipmove(cv->raddr, IPnoaddr);
    cv->lport = 0;
    cv->rport = 0;

    cv->r = nil;
    cv->rgen = 0;

    cv->restricted = false;
    cv->maxfragsize = 0;
    cv->ttl = MAXTTL;

    qreopen(cv->rq);
    qreopen(cv->wq);
    qreopen(cv->eq);

    qunlock(cv);
    return cv;
}
/*e: function Fsprotoclone */

/*s: function Fsconnected */
int
Fsconnected(Conv* c, char* msg)
{
    if(msg != nil && *msg != '\0')
        strncpy(c->cerr, msg, ERRMAX-1);

    switch(c->state){
    case Connecting:
        c->state = Connected;
        break;
    /*s: [[Fsconnected()]] switch state cases */
    case Announcing:
        c->state = Announced;
        break;
    /*e: [[Fsconnected()]] switch state cases */
    }
    wakeup(&c->cr);
    return 0;
}
/*e: function Fsconnected */

/*s: function Fsrcvpcol */
Proto*
Fsrcvpcol(Fs* f, uchar proto)
{
   return f->t2p[proto];
}
/*e: function Fsrcvpcol */

/*s: function Fsrcvpcolx */
Proto*
Fsrcvpcolx(Fs *f, uchar proto)
{
    return f->t2p[proto];
}
/*e: function Fsrcvpcolx */

/*s: function Fsnewcall */
/*
 *  called with protocol locked
 */
Conv*
Fsnewcall(Conv *c, uchar *raddr, ushort rport, uchar *laddr, ushort lport, uchar version)
{
    Conv *nc;
    Conv **l;
    int i;

    qlock(c);
    i = 0;
    for(l = &c->incall; *l; l = &(*l)->next)
        i++;
    if(i >= Maxincall) {
        static bool beenhere;

        qunlock(c);
        if (!beenhere) {
            beenhere = true;
            print("Fsnewcall: incall queue full (%d) on port %d\n",
                i, c->lport);
        }
        return nil;
    }

    /* find a free conversation */
    nc = Fsprotoclone(c->p, network);
    if(nc == nil) {
        qunlock(c);
        return nil;
    }
    ipmove(nc->raddr, raddr);
    nc->rport = rport;
    ipmove(nc->laddr, laddr);
    nc->lport = lport;

    nc->next = nil;
    *l = nc;

    nc->state = Connected;
    nc->ipversion = version;

    qunlock(c);

    wakeup(&c->listenr);

    return nc;
}
/*e: function Fsnewcall */

/*s: function ndbwrite */
long
ndbwrite(Fs *f, char *a, ulong off, int n)
{
    if(off > strlen(f->ndb))
        error(Eio);
    if(off+n >= sizeof(f->ndb))
        error(Eio);
    memmove(f->ndb+off, a, n);
    f->ndb[off+n] = 0;
    f->ndbvers++;
    f->ndbmtime = seconds();
    return n;
}
/*e: function ndbwrite */

/*s: function scalednconv */
ulong
scalednconv(void)
{
    if(cpuserver && conf.npage*BY2PG >= 128*MB)
        return Nchans*4;
    return Nchans;
}
/*e: function scalednconv */
/*e: kernel/network/ip/devip.c */
