/*s: networking/ndb/dnsdebug.c */
#include <u.h>
#include <libc.h>
#include <bio.h>
#include <ctype.h>
#include <ip.h>
#include <ndb.h>
#include "dns.h"

/*s: enum [[_anon_ (networking/ndb/dnsdebug.c)]] */
enum {
    Maxrequest=		128,
};
/*e: enum [[_anon_ (networking/ndb/dnsdebug.c)]] */

/*s: global [[cfg]]([[(networking/ndb/dnsdebug.c)]]) */
Cfg cfg;
/*e: global [[cfg]]([[(networking/ndb/dnsdebug.c)]]) */

/*s: global [[servername]] */
static char *servername;
/*e: global [[servername]] */
/*s: global [[serverrr]] */
static RR *serverrr;
/*e: global [[serverrr]] */
/*s: global [[serveraddrs]] */
static RR *serveraddrs;
/*e: global [[serveraddrs]] */

/*s: global [[dbfile]]([[(networking/ndb/dnsdebug.c)]]) */
char	*dbfile;
/*e: global [[dbfile]]([[(networking/ndb/dnsdebug.c)]]) */
/*s: global [[debug]]([[(networking/ndb/dnsdebug.c)]]) */
int	debug;
/*e: global [[debug]]([[(networking/ndb/dnsdebug.c)]]) */
/*s: global [[ipaddr]]([[(networking/ndb/dnsdebug.c)]]) */
ipaddr	ipaddr_;	/* my ip address */
/*e: global [[ipaddr]]([[(networking/ndb/dnsdebug.c)]]) */
/*s: global [[logfile]]([[(networking/ndb/dnsdebug.c)]]) */
char	*logfile = "dnsdebug";
/*e: global [[logfile]]([[(networking/ndb/dnsdebug.c)]]) */
/*s: global [[maxage]]([[(networking/ndb/dnsdebug.c)]]) */
int	maxage  = 60*60;
/*e: global [[maxage]]([[(networking/ndb/dnsdebug.c)]]) */
/*s: global [[mntpt]]([[(networking/ndb/dnsdebug.c)]]) */
char	mntpt[Maxpath];
/*e: global [[mntpt]]([[(networking/ndb/dnsdebug.c)]]) */
/*s: global [[needrefresh]]([[(networking/ndb/dnsdebug.c)]]) */
int	needrefresh;
/*e: global [[needrefresh]]([[(networking/ndb/dnsdebug.c)]]) */
/*s: global [[now]]([[(networking/ndb/dnsdebug.c)]]) */
ulong	now;
/*e: global [[now]]([[(networking/ndb/dnsdebug.c)]]) */
/*s: global [[nowns]]([[(networking/ndb/dnsdebug.c)]]) */
vlong	nowns;
/*e: global [[nowns]]([[(networking/ndb/dnsdebug.c)]]) */
/*s: global [[testing]]([[(networking/ndb/dnsdebug.c)]]) */
int	testing;
/*e: global [[testing]]([[(networking/ndb/dnsdebug.c)]]) */
/*s: global [[trace]]([[(networking/ndb/dnsdebug.c)]]) */
char	*trace;
/*e: global [[trace]]([[(networking/ndb/dnsdebug.c)]]) */
/*s: global [[traceactivity]]([[(networking/ndb/dnsdebug.c)]]) */
int	traceactivity;
/*e: global [[traceactivity]]([[(networking/ndb/dnsdebug.c)]]) */
/*s: global [[zonerefreshprogram]]([[(networking/ndb/dnsdebug.c)]]) */
char	*zonerefreshprogram;
/*e: global [[zonerefreshprogram]]([[(networking/ndb/dnsdebug.c)]]) */

void	docmd(int, char**);
void	doquery(char*, char*);
void	preloadserveraddrs(void);
int	prettyrrfmt(Fmt*);
int	setserver(char*);
void	squirrelserveraddrs(void);

/*s: function [[usage]]([[(networking/ndb/dnsdebug.c)]]) */
void
usage(void)
{
    fprint(2, "%s: [-rx] [-f db-file] [[@server] domain [type]]\n", argv0);
    exits("usage");
}
/*e: function [[usage]]([[(networking/ndb/dnsdebug.c)]]) */

/*s: function [[main]]([[(networking/ndb/dnsdebug.c)]]) */
void
main(int argc, char *argv[])
{
    int n;
    Biobuf in;
    char *p;
    char *f[4];

    strcpy(mntpt, "/net");
    cfg.inside = 1;

    ARGBEGIN{
    case 'f':
        dbfile = EARGF(usage());
        break;
    case 'r':
        cfg.resolver = 1;
        break;
    case 'x':
        dbfile = "/lib/ndb/external";
        strcpy(mntpt, "/net.alt");
        break;
    default:
        usage();
    }ARGEND

    now = time(nil);
    nowns = nsec();
    dninit();
    fmtinstall('R', prettyrrfmt);
    if(myipaddr(ipaddr_, mntpt) < 0)
        sysfatal("can't read my ip address");
    opendatabase();

    if(cfg.resolver)
        squirrelserveraddrs();

    debug = 1;

    if(argc > 0){
        docmd(argc, argv);
        exits(0);
    }

    Binit(&in, 0, OREAD);
    for(print("> "); p = Brdline(&in, '\n'); print("> ")){
        p[Blinelen(&in)-1] = 0;
        n = tokenize(p, f, 3);
        if(n>=1) {
            dnpurge();		/* flush the cache */
            docmd(n, f);
        }
    }
    exits(0);
}
/*e: function [[main]]([[(networking/ndb/dnsdebug.c)]]) */

/*s: function [[longtime]] */
static char*
longtime(long t)
{
    int d, h, m, n;
    static char x[128];

    for(d = 0; t >= 24*60*60; t -= 24*60*60)
        d++;
    for(h = 0; t >= 60*60; t -= 60*60)
        h++;
    for(m = 0; t >= 60; t -= 60)
        m++;
    n = 0;
    if(d)
        n += sprint(x, "%d day ", d);
    if(h)
        n += sprint(x+n, "%d hr ", h);
    if(m)
        n += sprint(x+n, "%d min ", m);
    if(t || n == 0)
        sprint(x+n, "%ld sec", t);
    return x;
}
/*e: function [[longtime]] */

/*s: function [[prettyrrfmt]] */
int
prettyrrfmt(Fmt *f)
{
    RR *rp;
    char buf[3*Domlen];
    char *p, *e;
    Txt *t;

    rp = va_arg(f->args, RR*);
    if(rp == 0){
        strcpy(buf, "<null>");
        goto out;
    }

    p = buf;
    e = buf + sizeof(buf);
    p = seprint(p, e, "%-32.32s %-15.15s %-5.5s", rp->owner->name,
        longtime(rp->db? rp->ttl: (rp->ttl - now)),
        rrname(rp->type, buf, sizeof buf));

    if(rp->negative){
        seprint(p, e, "negative rcode %d", rp->negrcode);
        goto out;
    }

    switch(rp->type){
    case Thinfo:
        seprint(p, e, "\t%s %s", rp->cpu->name, rp->os->name);
        break;
    case Tcname:
    case Tmb:
    case Tmd:
    case Tmf:
    case Tns:
        seprint(p, e, "\t%s", (rp->host? rp->host->name: ""));
        break;
    case Tmg:
    case Tmr:
        seprint(p, e, "\t%s", (rp->mb? rp->mb->name: ""));
        break;
    case Tminfo:
        seprint(p, e, "\t%s %s", (rp->mb? rp->mb->name: ""),
            (rp->rmb? rp->rmb->name: ""));
        break;
    case Tmx:
        seprint(p, e, "\t%lud %s", rp->pref,
            (rp->host? rp->host->name: ""));
        break;
    case Ta:
    case Taaaa:
        seprint(p, e, "\t%s", (rp->ip? rp->ip->name: ""));
        break;
    case Tptr:
        seprint(p, e, "\t%s", (rp->ptr? rp->ptr->name: ""));
        break;
    case Tsoa:
        seprint(p, e, "\t%s %s %lud %lud %lud %lud %lud",
            rp->host->name, rp->rmb->name, rp->soa->serial,
            rp->soa->refresh, rp->soa->retry,
            rp->soa->expire, rp->soa->minttl);
        break;
    case Tsrv:
        seprint(p, e, "\t%ud %ud %ud %s",
            rp->srv->pri, rp->srv->weight, rp->port, rp->host->name);
        break;
    case Tnull:
        seprint(p, e, "\t%.*H", rp->null->dlen, rp->null->data);
        break;
    case Ttxt:
        p = seprint(p, e, "\t");
        for(t = rp->txt; t != nil; t = t->next)
            p = seprint(p, e, "%s", t->p);
        break;
    case Trp:
        seprint(p, e, "\t%s %s", rp->rmb->name, rp->rp->name);
        break;
    case Tkey:
        seprint(p, e, "\t%d %d %d", rp->key->flags, rp->key->proto,
            rp->key->alg);
        break;
    case Tsig:
        seprint(p, e, "\t%d %d %d %lud %lud %lud %d %s",
            rp->sig->type, rp->sig->alg, rp->sig->labels,
            rp->sig->ttl, rp->sig->exp, rp->sig->incep,
            rp->sig->tag, rp->sig->signer->name);
        break;
    case Tcert:
        seprint(p, e, "\t%d %d %d",
            rp->sig->type, rp->sig->tag, rp->sig->alg);
        break;
    }
out:
    return fmtstrcpy(f, buf);
}
/*e: function [[prettyrrfmt]] */

/*s: function [[logsection]] */
void
logsection(char *flag, RR *rp)
{
    if(rp == nil)
        return;
    print("\t%s%R\n", flag, rp);
    for(rp = rp->next; rp != nil; rp = rp->next)
        print("\t      %R\n", rp);
}
/*e: function [[logsection]] */

/*s: function [[logreply]]([[(networking/ndb/dnsdebug.c)]]) */
void
logreply(int id, uchar *addr, DNSmsg *mp)
{
    RR *rp;
    char buf[12], resp[32];

    switch(mp->flags & Rmask){
    case Rok:
        strcpy(resp, "OK");
        break;
    case Rformat:
        strcpy(resp, "Format error");
        break;
    case Rserver:
        strcpy(resp, "Server failed");
        break;
    case Rname:
        strcpy(resp, "Nonexistent");
        break;
    case Runimplimented:
        strcpy(resp, "Unimplemented");
        break;
    case Rrefused:
        strcpy(resp, "Refused");
        break;
    default:
        sprint(resp, "%d", mp->flags & Rmask);
        break;
    }

    print("%d: rcvd %s from %I (%s%s%s%s%s)\n", id, resp, addr,
        mp->flags & Fauth? "authoritative": "",
        mp->flags & Ftrunc? " truncated": "",
        mp->flags & Frecurse? " recurse": "",
        mp->flags & Fcanrec? " can_recurse": "",
        (mp->flags & (Fauth|Rmask)) == (Fauth|Rname)? " nx": "");
    for(rp = mp->qd; rp != nil; rp = rp->next)
        print("\tQ:    %s %s\n", rp->owner->name,
            rrname(rp->type, buf, sizeof buf));
    logsection("Ans:  ", mp->an);
    logsection("Auth: ", mp->ns);
    logsection("Hint: ", mp->ar);
}
/*e: function [[logreply]]([[(networking/ndb/dnsdebug.c)]]) */

/*s: function [[logsend]]([[(networking/ndb/dnsdebug.c)]]) */
void
logsend(int id, int subid, uchar *addr, char *sname, char *rname, int type)
{
    char buf[12];

    print("%d.%d: sending to %I/%s %s %s\n", id, subid,
        addr, sname, rname, rrname(type, buf, sizeof buf));
}
/*e: function [[logsend]]([[(networking/ndb/dnsdebug.c)]]) */

/*s: function [[getdnsservers]]([[(networking/ndb/dnsdebug.c)]]) */
RR*
getdnsservers(int class)
{
    RR *rr;

    if(servername == nil)
        return dnsservers(class);

    rr = rralloc(Tns);
    rr->owner = dnlookup("local#dns#servers", class, 1);
    rr->host = dnlookup(servername, class, 1);

    return rr;
}
/*e: function [[getdnsservers]]([[(networking/ndb/dnsdebug.c)]]) */

/*s: function [[squirrelserveraddrs]] */
void
squirrelserveraddrs(void)
{
    int v4;
    char *attr;
    RR *rr, *rp, **l;
    Request req;

    /* look up the resolver address first */
    cfg.resolver = 0;
    debug = 0;
    if(serveraddrs)
        rrfreelist(serveraddrs);
    serveraddrs = nil;
    rr = getdnsservers(Cin);
    l = &serveraddrs;
    for(rp = rr; rp != nil; rp = rp->next){
        attr = ipattr(rp->host->name);
        v4 = strcmp(attr, "ip") == 0;
        if(v4 || strcmp(attr, "ipv6") == 0){
            *l = rralloc(v4? Ta: Taaaa);
            (*l)->owner = rp->host;
            (*l)->ip = rp->host;
            l = &(*l)->next;
            continue;
        }
        memset(&req, 0, sizeof req);
        req.isslave = 1;
        req.aborttime = NS2MS(nowns) + Maxreqtm;
        *l = dnresolve(rp->host->name, Cin, Ta, &req, 0, 0, Recurse, 0, 0);
        if(*l == nil)
            *l = dnresolve(rp->host->name, Cin, Taaaa, &req,
                0, 0, Recurse, 0, 0);
        while(*l != nil)
            l = &(*l)->next;
    }
    cfg.resolver = 1;
    debug = 1;
}
/*e: function [[squirrelserveraddrs]] */

/*s: function [[preloadserveraddrs]] */
void
preloadserveraddrs(void)
{
    RR *rp, **l, *first;

    l = &first;
    for(rp = serveraddrs; rp != nil; rp = rp->next){
        lock(&dnlock);
        rrcopy(rp, l);
        unlock(&dnlock);
        rrattach(first, Authoritative);
    }
}
/*e: function [[preloadserveraddrs]] */

/*s: function [[setserver]] */
int
setserver(char *server)
{
    if(servername != nil){
        free(servername);
        servername = nil;
        cfg.resolver = 0;
    }
    if(server == nil || *server == 0)
        return 0;
    servername = strdup(server);
    squirrelserveraddrs();
    if(serveraddrs == nil){
        print("can't resolve %s\n", servername);
        cfg.resolver = 0;
    } else
        cfg.resolver = 1;
    return cfg.resolver? 0: -1;
}
/*e: function [[setserver]] */

/*s: function [[doquery]] */
void
doquery(char *name, char *tstr)
{
    int len, type, rooted;
    char *p, *np;
    char buf[1024];
    RR *rr, *rp;
    Request req;

    if(cfg.resolver)
        preloadserveraddrs();

    /* default to an "ip" request if alpha, "ptr" if numeric */
    if(tstr == nil || *tstr == 0)
        if(strcmp(ipattr(name), "ip") == 0)
            tstr = "ptr";
        else
            tstr = "ip";

    /* if name end in '.', remove it */
    len = strlen(name);
    if(len > 0 && name[len-1] == '.'){
        rooted = 1;
        name[len-1] = 0;
    } else
        rooted = 0;

    /* inverse queries may need to be permuted */
    strncpy(buf, name, sizeof buf);
    if(strcmp("ptr", tstr) == 0 && cistrstr(name, ".arpa") == nil){
        /* TODO: reversing v6 addrs is harder */
        for(p = name; *p; p++)
            ;
        *p = '.';
        np = buf;
        len = 0;
        while(p >= name){
            len++;
            p--;
            if(*p == '.'){
                memmove(np, p+1, len);
                np += len;
                len = 0;
            }
        }
        memmove(np, p+1, len);
        np += len;
        strcpy(np, "in-addr.arpa");	/* TODO: ip6.arpa for v6 */
    }

    /* look it up */
    type = rrtype(tstr);
    if(type < 0){
        print("!unknown type %s\n", tstr);
        return;
    }

    memset(&req, 0, sizeof req);
    getactivity(&req, 0);
    req.isslave = 1;
    req.aborttime = NS2MS(nowns) + Maxreqtm;
    rr = dnresolve(buf, Cin, type, &req, 0, 0, Recurse, rooted, 0);
    if(rr){
        print("----------------------------\n");
        for(rp = rr; rp; rp = rp->next)
            print("answer %R\n", rp);
        print("----------------------------\n");
    }
    rrfreelist(rr);

    putactivity(0);
}
/*e: function [[doquery]] */

/*s: function [[docmd]] */
void
docmd(int n, char **f)
{
    int tmpsrv;
    char *name, *type;

    name = type = nil;
    tmpsrv = 0;

    if(*f[0] == '@') {
        if(setserver(f[0]+1) < 0)
            return;

        switch(n){
        case 3:
            type = f[2];
            /* fall through */
        case 2:
            name = f[1];
            tmpsrv = 1;
            break;
        }
    } else
        switch(n){
        case 2:
            type = f[1];
            /* fall through */
        case 1:
            name = f[0];
            break;
        }

    if(name == nil)
        return;

    doquery(name, type);

    if(tmpsrv)
        setserver("");
}
/*e: function [[docmd]] */
/*e: networking/ndb/dnsdebug.c */
