/*s: mnt.c */
/*s: kernel basic includes */
#include <u.h>
#include "../port/lib.h"
#include "../port/error.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
/*e: kernel basic includes */

// this used to be in devmnt.c, but to avoid backward deps I've splitted
// this file in 2 (which forced to put more stuff in portdat_files.h though).

/*s: global mntalloc */
struct Mntalloc mntalloc;
/*e: global mntalloc */

#define MAXRPC (IOHDRSZ+8192)

/*s: mnt.c Exxx errors */
char   Esbadstat[] = "invalid directory entry received from server";
char   Enoversion[] = "version not established for mount channel";
/*e: mnt.c Exxx errors */

/*s: function freetag */
void
freetag(int t)
{
    mntalloc.tagmask[t>>TAGSHIFT] &= ~(1<<(t&TAGMASK));
}
/*e: function freetag */

/*s: function mntfree */
void
mntfree(Mntrpc *r)
{
    if(r->b != nil)
        freeblist(r->b);
    lock(&mntalloc);
    if(mntalloc.nrpcfree >= 10){
        free(r->rpc);
        freetag(r->request.tag);
        free(r);
    }
    else{
        r->list = mntalloc.rpcfree;
        mntalloc.rpcfree = r;
        mntalloc.nrpcfree++;
    }
    mntalloc.nrpcused--;
    unlock(&mntalloc);
}
/*e: function mntfree */

/*s: function mntpntfree */
void
mntpntfree(Mnt *m)
{
    Mnt *f, **l;
    Queue *q;

    lock(&mntalloc);
    l = &mntalloc.list;
    for(f = *l; f; f = f->list) {
        if(f == m) {
            *l = m->list;
            break;
        }
        l = &f->list;
    }
    m->list = mntalloc.mntfree;
    mntalloc.mntfree = m;
    q = m->q;
    unlock(&mntalloc);

    qfree(q);
}
/*e: function mntpntfree */

/*s: function muxclose */
void
muxclose(Mnt *m)
{
    Mntrpc *q, *r;

    for(q = m->queue; q; q = r) {
        r = q->list;
        mntfree(q);
    }
    m->id = 0;
    free(m->version);
    m->version = nil;
    mntpntfree(m);
}
/*e: function muxclose */

/*s: function mntversion */
/*
 * Version is not multiplexed: message sent only once per connection.
 */
long
mntversion(Chan *c, char *version, int msize, int returnlen)
{
    Fcall f;
    uchar *msg;
    Mnt *m;
    char *v;
    long k, l;
    uvlong oo;
    char buf[128];

    qlock(&c->umqlock); /* make sure no one else does this until we've established ourselves */
    if(waserror()){
        qunlock(&c->umqlock);
        nexterror();
    }

    /* defaults */
    if(msize == 0)
        msize = MAXRPC;
    if(msize > c->iounit && c->iounit != 0)
        msize = c->iounit;
    v = version;
    if(v == nil || v[0] == '\0')
        v = VERSION9P;

    /* validity */
    if(msize < 0)
        error("bad iounit in version call");
    if(strncmp(v, VERSION9P, strlen(VERSION9P)) != 0)
        error("bad 9P version specification");

    m = c->mux;

    if(m != nil){
        qunlock(&c->umqlock);
        poperror();

        strecpy(buf, buf+sizeof buf, m->version);
        k = strlen(buf);
        if(strncmp(buf, v, k) != 0){
            snprint(buf, sizeof buf, "incompatible 9P versions %s %s", m->version, v);
            error(buf);
        }
        if(returnlen > 0){
            if(returnlen < k)
                error(Eshort);
            memmove(version, buf, k);
        }
        return k;
    }

    f.type = Tversion;
    f.tag = NOTAG;
    f.msize = msize;
    f.version = v;
    msg = malloc(8192+IOHDRSZ);
    if(msg == nil)
        exhausted("version memory");
    if(waserror()){
        free(msg);
        nexterror();
    }
    k = convS2M(&f, msg, 8192+IOHDRSZ);
    if(k == 0)
        error("bad fversion conversion on send");

    lock(c);
    oo = c->offset;
    c->offset += k;
    unlock(c);

    l = devtab[c->type]->write(c, msg, k, oo);

    if(l < k){
        lock(c);
        c->offset -= k - l;
        unlock(c);
        error("short write in fversion");
    }

    /* message sent; receive and decode reply */
    k = devtab[c->type]->read(c, msg, 8192+IOHDRSZ, c->offset);
    if(k <= 0)
        error("EOF receiving fversion reply");

    lock(c);
    c->offset += k;
    unlock(c);

    l = convM2S(msg, k, &f);
    if(l != k)
        error("bad fversion conversion on reply");
    if(f.type != Rversion){
        if(f.type == Rerror)
            error(f.ename);
        error("unexpected reply type in fversion");
    }
    if(f.msize > msize)
        error("server tries to increase msize in fversion");
    if(f.msize<256 || f.msize>1024*1024)
        error("nonsense value of msize in fversion");
    k = strlen(f.version);
    if(strncmp(f.version, v, k) != 0)
        error("bad 9P version returned from server");

    /* now build Mnt associated with this connection */
    lock(&mntalloc);
    m = mntalloc.mntfree;
    if(m != 0)
        mntalloc.mntfree = m->list;
    else {
        m = malloc(sizeof(Mnt));
        if(m == 0) {
            unlock(&mntalloc);
            exhausted("mount devices");
        }
    }
    m->list = mntalloc.list;
    mntalloc.list = m;
    m->version = nil;
    kstrdup(&m->version, f.version);
    m->id = mntalloc.id++;
    m->q = qopen(10*MAXRPC, 0, nil, nil);
    m->msize = f.msize;
    unlock(&mntalloc);

    if(returnlen > 0){
        if(returnlen < k)
            error(Eshort);
        memmove(version, f.version, k);
    }

    poperror(); /* msg */
    free(msg);

    lock(m);
    m->queue = 0;
    m->rip = 0;

    c->flag |= CMSG;
    c->mux = m;
    m->c = c;
    unlock(m);

    poperror(); /* c */
    qunlock(&c->umqlock);

    return k;
}
/*e: function mntversion */

/*e: mnt.c */
