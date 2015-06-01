/*s: kernel/network/ip/netlog.c */
#include    "u.h"
#include    "../port/lib.h"
#include    "mem.h"
#include    "dat.h"
#include    "fns.h"
#include    "../port/error.h"
#include    "../ip/ip.h"

/*s: enum _anon_ (kernel/network/ip/netlog.c) */
enum {
    Nlog        = 16*1024,
};
/*e: enum _anon_ (kernel/network/ip/netlog.c) */

/*s: struct Netlog */
/*
 *  action log
 */
struct Netlog {
    Lock;
    int opens;
    char*   buf;
    char    *end;
    char    *rptr;
    int len;

    int logmask;            /* mask of things to debug */
    uchar   iponly[IPaddrlen];      /* ip address to print debugging for */
    int iponlyset;

    QLock;
    Rendez;
};
/*e: struct Netlog */

/*s: struct Netlogflag */
typedef struct Netlogflag {
    char*   name;
    int mask;
} Netlogflag;
/*e: struct Netlogflag */

/*s: global flags */
static Netlogflag flags[] =
{
    { "ppp",    Logppp, },
    { "ip",     Logip, },
    { "fs",     Logfs, },
    { "tcp",    Logtcp, },
    { "icmp",   Logicmp, },
    { "udp",    Logudp, },
    { "compress",   Logcompress, },
    { "gre",    Loggre, },
    { "tcpwin", Logtcp|Logtcpwin, },
    { "tcprxmt",    Logtcp|Logtcprxmt, },
    { "udpmsg", Logudp|Logudpmsg, },
    { "ipmsg",  Logip|Logipmsg, },
    { "esp",    Logesp, },
    { nil,      0, },
};
/*e: global flags */

/*s: global Ebadnetctl */
char Ebadnetctl[] = "too few arguments for netlog control message";
/*e: global Ebadnetctl */

/*s: enum _anon_ (kernel/network/ip/netlog.c)2 */
enum
{
    CMset,
    CMclear,
    CMonly,
};
/*e: enum _anon_ (kernel/network/ip/netlog.c)2 */

/*s: global routecmd */
static
Cmdtab routecmd[] = {
    CMset,      "set",      0,
    CMclear,    "clear",    0,
    CMonly,     "only",     0,
};
/*e: global routecmd */

/*s: function netloginit */
void
netloginit(Fs *f)
{
    f->alog = smalloc(sizeof(Netlog));
}
/*e: function netloginit */

/*s: function netlogopen */
void
netlogopen(Fs *f)
{
    lock(f->alog);
    if(waserror()){
        unlock(f->alog);
        nexterror();
    }
    if(f->alog->opens == 0){
        if(f->alog->buf == nil)
            f->alog->buf = malloc(Nlog);
        if(f->alog->buf == nil)
            error(Enomem);
        f->alog->rptr = f->alog->buf;
        f->alog->end = f->alog->buf + Nlog;
    }
    f->alog->opens++;
    unlock(f->alog);
    poperror();
}
/*e: function netlogopen */

/*s: function netlogclose */
void
netlogclose(Fs *f)
{
    lock(f->alog);
    if(waserror()){
        unlock(f->alog);
        nexterror();
    }
    f->alog->opens--;
    if(f->alog->opens == 0){
        free(f->alog->buf);
        f->alog->buf = nil;
    }
    unlock(f->alog);
    poperror();
}
/*e: function netlogclose */

/*s: function netlogready */
static int
netlogready(void *a)
{
    Fs *f = a;

    return f->alog->len;
}
/*e: function netlogready */

/*s: function netlogread */
long
netlogread(Fs *f, void *a, ulong, long n)
{
    int i, d;
    char *p, *rptr;

    qlock(f->alog);
    if(waserror()){
        qunlock(f->alog);
        nexterror();
    }

    for(;;){
        lock(f->alog);
        if(f->alog->len){
            if(n > f->alog->len)
                n = f->alog->len;
            d = 0;
            rptr = f->alog->rptr;
            f->alog->rptr += n;
            if(f->alog->rptr >= f->alog->end){
                d = f->alog->rptr - f->alog->end;
                f->alog->rptr = f->alog->buf + d;
            }
            f->alog->len -= n;
            unlock(f->alog);

            i = n-d;
            p = a;
            memmove(p, rptr, i);
            memmove(p+i, f->alog->buf, d);
            break;
        }
        else
            unlock(f->alog);

        sleep(f->alog, netlogready, f);
    }

    qunlock(f->alog);
    poperror();

    return n;
}
/*e: function netlogread */

/*s: function netlogctl */
void
netlogctl(Fs *f, char* s, int n)
{
    int i, set;
    Netlogflag *fp;
    Cmdbuf *cb;
    Cmdtab *ct;

    cb = parsecmd(s, n);
    if(waserror()){
        free(cb);
        nexterror();
    }

    if(cb->nf < 2)
        error(Ebadnetctl);

    ct = lookupcmd(cb, routecmd, nelem(routecmd));

    SET(set);

    switch(ct->index){
    case CMset:
        set = 1;
        break;

    case CMclear:
        set = 0;
        break;

    case CMonly:
        parseip(f->alog->iponly, cb->f[1]);
        if(ipcmp(f->alog->iponly, IPnoaddr) == 0)
            f->alog->iponlyset = 0;
        else
            f->alog->iponlyset = 1;
        free(cb);
        poperror();
        return;

    default:
        cmderror(cb, "unknown netlog control message");
    }

    for(i = 1; i < cb->nf; i++){
        for(fp = flags; fp->name; fp++)
            if(strcmp(fp->name, cb->f[i]) == 0)
                break;
        if(fp->name == nil)
            continue;
        if(set)
            f->alog->logmask |= fp->mask;
        else
            f->alog->logmask &= ~fp->mask;
    }

    free(cb);
    poperror();
}
/*e: function netlogctl */

/*s: function netlog */
void
netlog(Fs *f, int mask, char *fmt, ...)
{
    char buf[256], *t, *fp;
    int i, n;
    va_list arg;

    if(!(f->alog->logmask & mask))
        return;

    if(f->alog->opens == 0)
        return;

    va_start(arg, fmt);
    n = vseprint(buf, buf+sizeof(buf), fmt, arg) - buf;
    va_end(arg);

    lock(f->alog);
    i = f->alog->len + n - Nlog;
    if(i > 0){
        f->alog->len -= i;
        f->alog->rptr += i;
        if(f->alog->rptr >= f->alog->end)
            f->alog->rptr = f->alog->buf + (f->alog->rptr - f->alog->end);
    }
    t = f->alog->rptr + f->alog->len;
    fp = buf;
    f->alog->len += n;
    while(n-- > 0){
        if(t >= f->alog->end)
            t = f->alog->buf + (t - f->alog->end);
        *t++ = *fp++;
    }
    unlock(f->alog);

    wakeup(f->alog);
}
/*e: function netlog */
/*e: kernel/network/ip/netlog.c */
