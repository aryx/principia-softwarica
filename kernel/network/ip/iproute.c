/*s: kernel/network/ip/iproute.c */
#include    "u.h"
#include    "../port/lib.h"
#include    "mem.h"
#include    "dat.h"
#include    "fns.h"
#include    "../port/error.h"

#include    "ip.h"

static void walkadd(Fs*, Route**, Route*);
static void addnode(Fs*, Route**, Route*);
static void calcd(Route*);

/*s: global [[v4freelist]] */
/* these are used for all instances of IP */
static Route*   v4freelist;
/*e: global [[v4freelist]] */
/*s: global [[v4routegeneration]] */
static ulong v4routegeneration;
/*e: global [[v4routegeneration]] */
/*s: global [[routelock]] */
static RWlock   routelock;
/*e: global [[routelock]] */

/*s: global [[v6freelist]] */
static Route*   v6freelist;
/*e: global [[v6freelist]] */
/*s: global [[v6routegeneration]] */
static ulong v6routegeneration;
/*e: global [[v6routegeneration]] */

/*s: function [[freeroute]] */
static void
freeroute(Route *r)
{
    Route **l;

    r->left = nil;
    r->right = nil;
    if(r->type & Rv4)
        l = &v4freelist;
    /*s: [[freeroute()]] if ipv6 route */
    else
        l = &v6freelist;
    /*e: [[freeroute()]] if ipv6 route */
    r->mid = *l;
    *l = r;
}
/*e: function [[freeroute]] */

/*s: function [[allocroute]] */
static Route*
allocroute(int type)
{
    Route *r;
    int n;
    Route **l;

    if(type & Rv4){
        n = sizeof(RouteTree) + sizeof(V4route);
        l = &v4freelist;
    }
    /*s: [[allocroute()]] if ipv6 route */
    else {
        n = sizeof(RouteTree) + sizeof(V6route);
        l = &v6freelist;
    }
    /*e: [[allocroute()]] if ipv6 route */

    r = *l;
    if(r != nil){
        *l = r->mid;
    } else {
        r = malloc(n);
        if(r == nil)
            panic("out of routing nodes");
    }
    memset(r, 0, n);

    r->type = type;
    r->ifc = nil;
    r->ref = 1;

    return r;
}
/*e: function [[allocroute]] */

/*s: function [[addqueue]] */
static void
addqueue(Route **q, Route *r)
{
    Route *l;

    if(r == nil)
        return;

    l = allocroute(r->type);
    l->mid = *q;
    *q = l;
    l->left = r;
}
/*e: function [[addqueue]] */

/*s: function [[lcmp]] */
/*
 *   compare 2 v6 addresses
 */
static int
lcmp(ulong *a, ulong *b)
{
    int i;

    for(i = 0; i < IPllen; i++){
        if(a[i] > b[i])
            return 1;
        if(a[i] < b[i])
            return -1;
    }
    return 0;
}
/*e: function [[lcmp]] */

/*s: enum [[_anon_ (kernel/network/ip/iproute.c)]] */
/*
 *  compare 2 v4 or v6 ranges
 */
enum
{
    Rpreceeds,
    Rfollows,
    Requals,
    Rcontains,
    Rcontained,
};
/*e: enum [[_anon_ (kernel/network/ip/iproute.c)]] */

/*s: function [[rangecompare]] */
static int
rangecompare(Route *a, Route *b)
{
    if(a->type & Rv4){
        if(a->v4.endaddress < b->v4.address)
            return Rpreceeds;

        if(a->v4.address > b->v4.endaddress)
            return Rfollows;

        if(a->v4.address <= b->v4.address
        && a->v4.endaddress >= b->v4.endaddress){

            if(a->v4.address == b->v4.address
            && a->v4.endaddress == b->v4.endaddress)
                return Requals;
            return Rcontains;
        }
        return Rcontained;
    }
    /*s: [[rangecompare()]] if ipv6 routes */
    if(lcmp(a->v6.endaddress, b->v6.address) < 0)
        return Rpreceeds;

    if(lcmp(a->v6.address, b->v6.endaddress) > 0)
        return Rfollows;

    if(lcmp(a->v6.address, b->v6.address) <= 0
    && lcmp(a->v6.endaddress, b->v6.endaddress) >= 0){
        if(lcmp(a->v6.address, b->v6.address) == 0
        && lcmp(a->v6.endaddress, b->v6.endaddress) == 0)
                return Requals;
        return Rcontains;
    }

    return Rcontained;
    /*e: [[rangecompare()]] if ipv6 routes */
}
/*e: function [[rangecompare]] */

/*s: function [[copygate]] */
static void
copygate(Route *old, Route *new)
{
    if(new->type & Rv4)
        memmove(old->v4.gate, new->v4.gate, IPv4addrlen);
    else
        memmove(old->v6.gate, new->v6.gate, IPaddrlen);
}
/*e: function [[copygate]] */

/*s: function [[walkadd]] */
/*
 *  walk down a tree adding nodes back in
 */
static void
walkadd(Fs *f, Route **root, Route *p)
{
    Route *l, *r;

    l = p->left;
    r = p->right;
    p->left = 0;
    p->right = 0;
    addnode(f, root, p);
    if(l)
        walkadd(f, root, l);
    if(r)
        walkadd(f, root, r);
}
/*e: function [[walkadd]] */

/*s: function [[calcd]] */
/*
 *  calculate depth
 */
static void
calcd(Route *p)
{
    Route *q;
    int d;

    if(p) {
        d = 0;
        q = p->left;
        if(q)
            d = q->depth;
        q = p->right;
        if(q && q->depth > d)
            d = q->depth;
        q = p->mid;
        if(q && q->depth > d)
            d = q->depth;
        p->depth = d+1;
    }
}
/*e: function [[calcd]] */

/*s: function [[balancetree]] */
/*
 *  balance the tree at the current node
 */
static void
balancetree(Route **cur)
{
    Route *p, *l, *r;
    int dl, dr;

    /*
     * if left and right are
     * too out of balance,
     * rotate tree node
     */
    p = *cur;
    dl = 0; if(l = p->left) dl = l->depth;
    dr = 0; if(r = p->right) dr = r->depth;

    if(dl > dr+1) {
        p->left = l->right;
        l->right = p;
        *cur = l;
        calcd(p);
        calcd(l);
    } else
    if(dr > dl+1) {
        p->right = r->left;
        r->left = p;
        *cur = r;
        calcd(p);
        calcd(r);
    } else
        calcd(p);
}
/*e: function [[balancetree]] */

/*s: function [[addnode]] */
/*
 *  add a new node to the tree
 */
static void
addnode(Fs *f, Route **cur, Route *new)
{
    Route *p;

    p = *cur;
    if(p == 0) {
        *cur = new;
        new->depth = 1;
        return;
    }

    switch(rangecompare(new, p)){
    case Rpreceeds:
        addnode(f, &p->left, new);
        break;
    case Rfollows:
        addnode(f, &p->right, new);
        break;
    case Rcontains:
        /*
         *  if new node is superset
         *  of tree node,
         *  replace tree node and
         *  queue tree node to be
         *  merged into root.
         */
        *cur = new;
        new->depth = 1;
        addqueue(&f->queue, p);
        break;
    case Requals:
        /*
         *  supercede the old entry if the old one isn't
         *  a local interface.
         */
        if((p->type & Rifc) == 0){
            p->type = new->type;
            p->ifcid = -1;
            copygate(p, new);
        } else if(new->type & Rifc)
            p->ref++;
        freeroute(new);
        break;
    case Rcontained:
        addnode(f, &p->mid, new);
        break;
    }

    balancetree(cur);
}
/*e: function [[addnode]] */

/*s: macro [[V4H]] */
#define V4H(a)  ((a&0x07ffffff)>>(32-Lroot-5))
/*e: macro [[V4H]] */

/*s: function [[v4addroute]] */
void
v4addroute(Fs *f, char *tag, ipv4 a, ipv4 mask, ipv4 gate, int type)
{
    Route *p;
    ulong m;
    ulong sa; // start address
    ulong ea; // end address
    int h, eh;

    m = nhgetl(mask);
    sa = nhgetl(a) & m;
    ea = sa | ~m;

    eh = V4H(ea);
    for(h=V4H(sa); h<=eh; h++) {
        p = allocroute(Rv4 | type);
        p->v4.address = sa;
        p->v4.endaddress = ea;
        memmove(p->v4.gate, gate, sizeof(p->v4.gate));
        memmove(p->tag, tag, sizeof(p->tag));

        wlock(&routelock);
        addnode(f, &f->v4root[h], p);
        /*s: [[v4addroute()]] if f has a route queue */
        while(p = f->queue) {
            f->queue = p->mid;
            walkadd(f, &f->v4root[h], p->left);
            freeroute(p);
        }
        /*e: [[v4addroute()]] if f has a route queue */
        wunlock(&routelock);
    }
    v4routegeneration++;

    ipifcaddroute(f, Rv4, a, mask, gate, type);
}
/*e: function [[v4addroute]] */

/*s: macro [[V6H]] */
#define V6H(a)  (((a)[IPllen-1] & 0x07ffffff)>>(32-Lroot-5))
/*e: macro [[V6H]] */
/*s: function [[v6addroute]] */
//#define ISDFLT(a, mask, tag) ((ipcmp((a),v6Unspecified)==0) && (ipcmp((mask),v6Unspecified)==0) && (strcmp((tag), "ra")!=0))

void
v6addroute(Fs *f, char *tag, uchar *a, uchar *mask, uchar *gate, int type)
{
    Route *p;
    ulong sa[IPllen], ea[IPllen];
    ulong x, y;
    int h, eh;

    /*
    if(ISDFLT(a, mask, tag))
        f->v6p->cdrouter = -1;
    */


    for(h = 0; h < IPllen; h++){
        x = nhgetl(a+4*h);
        y = nhgetl(mask+4*h);
        sa[h] = x & y;
        ea[h] = x | ~y;
    }

    eh = V6H(ea);
    for(h = V6H(sa); h <= eh; h++) {
        p = allocroute(type);
        memmove(p->v6.address, sa, IPaddrlen);
        memmove(p->v6.endaddress, ea, IPaddrlen);
        memmove(p->v6.gate, gate, IPaddrlen);
        memmove(p->tag, tag, sizeof(p->tag));

        wlock(&routelock);
        addnode(f, &f->v6root[h], p);
        while(p = f->queue) {
            f->queue = p->mid;
            walkadd(f, &f->v6root[h], p->left);
            freeroute(p);
        }
        wunlock(&routelock);
    }
    v6routegeneration++;

    ipifcaddroute(f, 0, a, mask, gate, type);
}
/*e: function [[v6addroute]] */

/*s: function [[looknode]] */
Route**
looknode(Route **cur, Route *r)
{
    Route *p;

    for(;;){
        p = *cur;
        if(p == 0)
            return 0;

        switch(rangecompare(r, p)){
        case Rcontains:
            return 0;
        case Rpreceeds:
            cur = &p->left;
            break;
        case Rfollows:
            cur = &p->right;
            break;
        case Rcontained:
            cur = &p->mid;
            break;
        case Requals:
            return cur;
        }
    }
}
/*e: function [[looknode]] */

/*s: function [[v4delroute]] */
void
v4delroute(Fs *f, ipv4 a, ipv4 mask, bool dolock)
{
    Route **r, *p;
    Route rt;
    int h, eh;
    ulong m;

    m = nhgetl(mask);
    rt.v4.address = nhgetl(a) & m;
    rt.v4.endaddress = rt.v4.address | ~m;
    rt.type = Rv4;

    eh = V4H(rt.v4.endaddress);
    for(h=V4H(rt.v4.address); h<=eh; h++) {
        if(dolock)
            wlock(&routelock);
        r = looknode(&f->v4root[h], &rt);
        if(r) {
            p = *r;
            if(--(p->ref) == 0){
                *r = 0;
                addqueue(&f->queue, p->left);
                addqueue(&f->queue, p->mid);
                addqueue(&f->queue, p->right);
                freeroute(p);
                while(p = f->queue) {
                    f->queue = p->mid;
                    walkadd(f, &f->v4root[h], p->left);
                    freeroute(p);
                }
            }
        }
        if(dolock)
            wunlock(&routelock);
    }
    v4routegeneration++;

    ipifcremroute(f, Rv4, a, mask);
}
/*e: function [[v4delroute]] */

/*s: function [[v6delroute]] */
void
v6delroute(Fs *f, uchar *a, uchar *mask, int dolock)
{
    Route **r, *p;
    Route rt;
    int h, eh;
    ulong x, y;

    for(h = 0; h < IPllen; h++){
        x = nhgetl(a+4*h);
        y = nhgetl(mask+4*h);
        rt.v6.address[h] = x & y;
        rt.v6.endaddress[h] = x | ~y;
    }
    rt.type = 0;

    eh = V6H(rt.v6.endaddress);
    for(h=V6H(rt.v6.address); h<=eh; h++) {
        if(dolock)
            wlock(&routelock);
        r = looknode(&f->v6root[h], &rt);
        if(r) {
            p = *r;
            if(--(p->ref) == 0){
                *r = 0;
                addqueue(&f->queue, p->left);
                addqueue(&f->queue, p->mid);
                addqueue(&f->queue, p->right);
                freeroute(p);
                while(p = f->queue) {
                    f->queue = p->mid;
                    walkadd(f, &f->v6root[h], p->left);
                    freeroute(p);
                }
            }
        }
        if(dolock)
            wunlock(&routelock);
    }
    v6routegeneration++;

    ipifcremroute(f, 0, a, mask);
}
/*e: function [[v6delroute]] */

/*s: function [[v4lookup]] */
Route*
v4lookup(Fs *f, ipv4 a, Conv *c)
{
    Route *p, *q;
    ulong la;
    /*s: [[v4lookup()]] locals */
    ipaddr gate;
    Ipifc *ifc;
    /*e: [[v4lookup()]] locals */

    /*s: [[v4lookup()]] return cached route if still valid route */
    if(c != nil && c->r != nil && c->r->ifc != nil && c->rgen == v4routegeneration)
        return c->r;
    /*e: [[v4lookup()]] return cached route if still valid route */

    la = nhgetl(a);
    q = nil;
    /*s: [[v4lookup()]] ternary search for route q for la in route forest */
    for(p=f->v4root[V4H(la)]; p;)
        if(la >= p->v4.address) {
            if(la <= p->v4.endaddress) {
                q = p;
                p = p->mid;
            } else
                p = p->right;
        } else
            p = p->left;
    /*e: [[v4lookup()]] ternary search for route q for la in route forest */
    /*s: [[v4lookup()]] make sure route q has an up to date ifc */
    if(q && (q->ifc == nil || q->ifcid != q->ifc->ifcid)){
        if(q->type & Rifc) {
            hnputl(gate+IPv4off, q->v4.address);
            memmove(gate, v4prefix, IPv4off);
        } else
            v4tov6(gate, q->v4.gate);

        ifc = findipifc(f, gate, q->type);
        if(ifc == nil)
            return nil;
        q->ifc = ifc;
        q->ifcid = ifc->ifcid;
    }
    /*e: [[v4lookup()]] make sure route q has an up to date ifc */

    if(c != nil){
        c->r = q;
        c->rgen = v4routegeneration;
    }

    return q;
}
/*e: function [[v4lookup]] */

/*s: function [[v6lookup]] */
Route*
v6lookup(Fs *f, uchar *a, Conv *c)
{
    Route *p, *q;
    ulong la[IPllen];
    int h;
    ulong x, y;
    uchar gate[IPaddrlen];
    Ipifc *ifc;

    if(memcmp(a, v4prefix, IPv4off) == 0){
        q = v4lookup(f, a+IPv4off, c);
        if(q != nil)
            return q;
    }

    if(c != nil && c->r != nil && c->r->ifc != nil && c->rgen == v6routegeneration)
        return c->r;

    for(h = 0; h < IPllen; h++)
        la[h] = nhgetl(a+4*h);

    q = 0;
    for(p=f->v6root[V6H(la)]; p;){
        for(h = 0; h < IPllen; h++){
            x = la[h];
            y = p->v6.address[h];
            if(x == y)
                continue;
            if(x < y){
                p = p->left;
                goto next;
            }
            break;
        }
        for(h = 0; h < IPllen; h++){
            x = la[h];
            y = p->v6.endaddress[h];
            if(x == y)
                continue;
            if(x > y){
                p = p->right;
                goto next;
            }
            break;
        }
        q = p;
        p = p->mid;
next:       ;
    }

    if(q && (q->ifc == nil || q->ifcid != q->ifc->ifcid)){
        if(q->type & Rifc) {
            for(h = 0; h < IPllen; h++)
                hnputl(gate+4*h, q->v6.address[h]);
            ifc = findipifc(f, gate, q->type);
        } else
            ifc = findipifc(f, q->v6.gate, q->type);
        if(ifc == nil)
            return nil;
        q->ifc = ifc;
        q->ifcid = ifc->ifcid;
    }
    if(c != nil){
        c->r = q;
        c->rgen = v6routegeneration;
    }

    return q;
}
/*e: function [[v6lookup]] */

/*s: function [[routetype]] */
void
routetype(int type, char *p)
{
    memset(p, ' ', 4);
    p[4] = 0;
    if(type & Rv4)
        *p++ = '4';
    else
        *p++ = '6';
    if(type & Rifc)
        *p++ = 'i';
    if(type & Runi)
        *p++ = 'u';
    else if(type & Rbcast)
        *p++ = 'b';
    else if(type & Rmulti)
        *p++ = 'm';
    if(type & Rptpt)
        *p = 'p';
}
/*e: function [[routetype]] */

/*s: global [[rformat]] */
static char *rformat = "%-15I %-4M %-15I %4.4s %4.4s %3s\n";
/*e: global [[rformat]] */

/*s: function [[convroute]] */
void
convroute(Route *r, uchar *addr, uchar *mask, uchar *gate, char *t, int *nifc)
{
    int i;

    if(r->type & Rv4){
        memmove(addr, v4prefix, IPv4off);
        hnputl(addr+IPv4off, r->v4.address);
        memset(mask, 0xff, IPv4off);
        hnputl(mask+IPv4off, ~(r->v4.endaddress ^ r->v4.address));
        memmove(gate, v4prefix, IPv4off);
        memmove(gate+IPv4off, r->v4.gate, IPv4addrlen);
    } else {
        for(i = 0; i < IPllen; i++){
            hnputl(addr + 4*i, r->v6.address[i]);
            hnputl(mask + 4*i, ~(r->v6.endaddress[i] ^ r->v6.address[i]));
        }
        memmove(gate, r->v6.gate, IPaddrlen);
    }

    routetype(r->type, t);

    if(r->ifc)
        *nifc = r->ifc->conv->x;
    else
        *nifc = -1;
}
/*e: function [[convroute]] */

/*s: function [[sprintroute]] */
/*
 *  this code is not in rr to reduce stack size
 */
static void
sprintroute(Route *r, Routewalk *rw)
{
    int nifc, n;
    char t[5], *iname, ifbuf[5];
    ipaddr addr, mask, gate;
    char *p;

    convroute(r, addr, mask, gate, t, &nifc);
    iname = "-";
    if(nifc != -1) {
        iname = ifbuf;
        snprint(ifbuf, sizeof ifbuf, "%d", nifc);
    }
    p = seprint(rw->p, rw->e, rformat, addr, mask, gate, t, r->tag, iname);
    if(rw->o < 0){
        n = p - rw->p;
        if(n > -rw->o){
            memmove(rw->p, rw->p-rw->o, n+rw->o);
            rw->p = p + rw->o;
        }
        rw->o += n;
    } else
        rw->p = p;
}
/*e: function [[sprintroute]] */

/*s: function [[rr]] */
/*
 *  recurse descending tree, applying the function in Routewalk
 */
static int
rr(Route *r, Routewalk *rw)
{
    int h;

    if(rw->e <= rw->p)
        return 0;
    if(r == nil)
        return 1;

    if(rr(r->left, rw) == 0)
        return 0;

    if(r->type & Rv4)
        h = V4H(r->v4.address);
    /*s: [[rr()]] else if ipv6 address */
    else
        h = V6H(r->v6.address);
    /*e: [[rr()]] else if ipv6 address */

    if(h == rw->h)
        rw->walk(r, rw);

    if(rr(r->mid, rw) == 0)
        return 0;

    return rr(r->right, rw);
}
/*e: function [[rr]] */

/*s: function [[ipwalkroutes]] */
void
ipwalkroutes(Fs *f, Routewalk *rw)
{
    rlock(&routelock);
    if(rw->e > rw->p) {
        for(rw->h = 0; rw->h < nelem(f->v4root); rw->h++)
            if(rr(f->v4root[rw->h], rw) == 0)
                break;
    }
    if(rw->e > rw->p) {
        for(rw->h = 0; rw->h < nelem(f->v6root); rw->h++)
            if(rr(f->v6root[rw->h], rw) == 0)
                break;
    }
    runlock(&routelock);
}
/*e: function [[ipwalkroutes]] */

/*s: function [[routeread]] */
long
routeread(Fs *f, char *p, ulong offset, int n)
{
    Routewalk rw;

    rw.p = p;
    rw.e = p+n;
    rw.o = -offset;
    rw.walk = sprintroute;

    ipwalkroutes(f, &rw);

    return rw.p - p;
}
/*e: function [[routeread]] */

/*s: function [[delroute]] */
/*
 *  this code is not in routeflush to reduce stack size
 */
void
delroute(Fs *f, Route *r, int dolock)
{
    ipaddr addr, mask, gate;
    char t[5];
    int nifc;

    convroute(r, addr, mask, gate, t, &nifc);
    if(r->type & Rv4)
        v4delroute(f, addr+IPv4off, mask+IPv4off, dolock);
    else
        v6delroute(f, addr, mask, dolock);
}
/*e: function [[delroute]] */

/*s: function [[routeflush]] */
/*
 *  recurse until one route is deleted
 *    returns 0 if nothing is deleted, 1 otherwise
 */
int
routeflush(Fs *f, Route *r, char *tag)
{
    if(r == nil)
        return 0;
    if(routeflush(f, r->mid, tag))
        return 1;
    if(routeflush(f, r->left, tag))
        return 1;
    if(routeflush(f, r->right, tag))
        return 1;
    if((r->type & Rifc) == 0){
        if(tag == nil || strncmp(tag, r->tag, sizeof(r->tag)) == 0){
            delroute(f, r, 0);
            return 1;
        }
    }
    return 0;
}
/*e: function [[routeflush]] */

/*s: function [[iproute]] */
Route *
iproute(Fs *fs, ipaddr ip)
{
    if(isv4(ip))
        return v4lookup(fs, ip+IPv4off, nil);
    else
        return v6lookup(fs, ip, nil);
}
/*e: function [[iproute]] */

/*s: function [[printroute]] */
static void
printroute(Route *r)
{
    int nifc;
    char t[5], *iname, ifbuf[5];
    ipaddr addr, mask, gate;

    convroute(r, addr, mask, gate, t, &nifc);
    iname = "-";
    if(nifc != -1) {
        iname = ifbuf;
        snprint(ifbuf, sizeof ifbuf, "%d", nifc);
    }
    print(rformat, addr, mask, gate, t, r->tag, iname);
}
/*e: function [[printroute]] */

/*s: function [[routewrite]] */
long
routewrite(Fs *f, Chan *c, char *p, int n)
{
    int h, changed;
    char *tag;
    Cmdbuf *cb;
    ipaddr addr, mask, gate;
    IPaux *a, *na;
    Route *q;

    cb = parsecmd(p, n);
    if(waserror()){
        free(cb);
        nexterror();
    }

    if(strcmp(cb->f[0], "flush") == 0){
        tag = cb->f[1];
        for(h = 0; h < nelem(f->v4root); h++)
            for(changed = 1; changed;){
                wlock(&routelock);
                changed = routeflush(f, f->v4root[h], tag);
                wunlock(&routelock);
            }
        for(h = 0; h < nelem(f->v6root); h++)
            for(changed = 1; changed;){
                wlock(&routelock);
                changed = routeflush(f, f->v6root[h], tag);
                wunlock(&routelock);
            }
    } else if(strcmp(cb->f[0], "remove") == 0){
        if(cb->nf < 3)
            error(Ebadarg);
        if (parseip(addr, cb->f[1]) == -1)
            error(Ebadip);
        parseipmask(mask, cb->f[2]);
        if(memcmp(addr, v4prefix, IPv4off) == 0)
            v4delroute(f, addr+IPv4off, mask+IPv4off, 1);
        else
            v6delroute(f, addr, mask, 1);
    } else if(strcmp(cb->f[0], "add") == 0){
        if(cb->nf < 4)
            error(Ebadarg);
        if(parseip(addr, cb->f[1]) == -1 ||
            parseip(gate, cb->f[3]) == -1)
            error(Ebadip);
        parseipmask(mask, cb->f[2]);
        tag = "none";
        if(c != nil){
            a = c->aux;
            tag = a->tag;
        }
        if(memcmp(addr, v4prefix, IPv4off) == 0)
            v4addroute(f, tag, addr+IPv4off, mask+IPv4off, gate+IPv4off, 0);
        else
            v6addroute(f, tag, addr, mask, gate, 0);
    } else if(strcmp(cb->f[0], "tag") == 0) {
        if(cb->nf < 2)
            error(Ebadarg);

        a = c->aux;
        na = newipaux(a->owner, cb->f[1]);
        c->aux = na;
        free(a);
    } else if(strcmp(cb->f[0], "route") == 0) {
        if(cb->nf < 2)
            error(Ebadarg);
        if (parseip(addr, cb->f[1]) == -1)
            error(Ebadip);

        q = iproute(f, addr);
        print("%I: ", addr);
        if(q == nil)
            print("no route\n");
        else
            printroute(q);
    }

    poperror();
    free(cb);
    return n;
}
/*e: function [[routewrite]] */
/*e: kernel/network/ip/iproute.c */
